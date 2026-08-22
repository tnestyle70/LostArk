#!/usr/bin/env python3
"""Build the four-character Effect V1 horizontal application ledger.

The ledger is evidence and application planning data.  It never writes an
Effect document or expands the material-program registry.  The current tuple
inventory is rebuilt in memory so a stale checked-in Track B artifact cannot
be mistaken for runtime proof.  Source material identities are joined only by
their exact committed ``sourceMaterialPath``.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import sys
import tempfile
from collections import Counter
from pathlib import Path
from typing import Any, Mapping


ROOT = Path(__file__).resolve().parents[2]
TOOLS_ROOT = ROOT / "Tools" / "EffectPipeline"
if str(TOOLS_ROOT) not in sys.path:
    sys.path.insert(0, str(TOOLS_ROOT))

import build_effect_material_program_registry as material_registry  # noqa: E402
import build_effect_tuple_cohort_inventory as tuple_inventory  # noqa: E402


SCHEMA = "lostark.four-character-effect-v1-horizontal-application"
FORMAT_VERSION = 1
LEDGER_ID = "effect.v1.horizontal.four-character.product.v1"

OUTPUT_PATH = (
    ROOT
    / "Data"
    / "Effects"
    / "Contracts"
    / "four-character-effect-v1-horizontal-application.v1.json"
)
SOURCE_CONTRACT_PATH = (
    ROOT
    / "Data"
    / "Effects"
    / "AuthoredCorrections"
    / "Generated"
    / "FourClassCombat.source-material-contract.json"
)
REGISTRY_PATH = (
    ROOT
    / "Data"
    / "Effects"
    / "MaterialPrograms"
    / "effect-material-program-registry.v1.json"
)
EFFECT_CATALOG_PATH = ROOT / "Data" / "Effects" / "EffectCatalog.json"
DATA_ROOT = ROOT / "Data"

TARGET_DOMAINS = ("Artist", "DimensionMaster", "LanceMaster", "Warlord")
TARGET_DOMAIN_SET = set(TARGET_DOMAINS)
HORIZONTAL_FINE_KINDS = (
    "SPRITE_PARTICLE",
    "MESH_PARTICLE",
    "DECAL_PARTICLE",
)
HORIZONTAL_FINE_KIND_SET = set(HORIZONTAL_FINE_KINDS)
FEATURE_DEFERRED_FINE_KINDS = (
    "LEGACY_STANDALONE_SPRITE",
    "AUTHORED_LEGACY_TRAIL",
    "CASCADE_RIBBON",
)
FEATURE_DEFERRED_FINE_KIND_SET = set(FEATURE_DEFERRED_FINE_KINDS)

APPLICATION_STATES = (
    "CURRENT_BOUND_INLINE_EXACT",
    "INLINE_MIRROR_CANDIDATE",
    "SOURCE_EXACT_SIMPLE_RT0_PACKET_PENDING",
    "SOURCE_EXACT_PACKET_PENDING",
    "PROJECT_RECONSTRUCTION_PENDING",
    "FEATURE_DEFERRED",
    "EVIDENCE_BLOCKED",
)
MANUAL_REVIEW = "PENDING"
MISMATCH_AXIS_ORDER = ("blendMode", "twoSided", "depthTest")

EXPECTED_PRODUCT_OCCURRENCE_COUNT = 1885
EXPECTED_HORIZONTAL_FINE_OCCURRENCE_COUNT = 1871
EXPECTED_FEATURE_DEFERRED_OCCURRENCE_COUNT = 14
EXPECTED_FINE_KIND_COUNTS = {
    "SPRITE_PARTICLE": 1337,
    "MESH_PARTICLE": 493,
    "DECAL_PARTICLE": 41,
    "LEGACY_STANDALONE_SPRITE": 2,
    "AUTHORED_LEGACY_TRAIL": 8,
    "CASCADE_RIBBON": 4,
}

SIMPLE_RT0_ADVANCED_BLOCKERS = {
    "OUTPUT_TOPOLOGY_MRT_UNPROVEN",
    "SCENE_INPUTS_UNPROVEN",
    "WPO_VERTEX_PROGRAM_UNPROVEN",
}


class ApplicationError(RuntimeError):
    """Raised when a ledger join or invariant is not exact."""


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ApplicationError(message)


def canonical_json_bytes(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")


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


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_json_bytes(value)).hexdigest()


def file_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def load_json(path: Path, label: str) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise ApplicationError(f"cannot read {label}: {path}: {exc}") from exc
    require(isinstance(value, dict), f"{label} root must be an object")
    return value


def relative_path(path: Path) -> str:
    return path.resolve().relative_to(ROOT.resolve()).as_posix()


def _exact_source_render_state(identity: Mapping[str, Any] | None) -> dict[str, Any] | None:
    if identity is None:
        return None
    state = identity.get("renderState")
    if not isinstance(state, dict):
        return None
    blend_mode = state.get("blendMode")
    two_sided = state.get("twoSided")
    disable_depth_test = state.get("disableDepthTest")
    if (
        not isinstance(blend_mode, str)
        or not isinstance(two_sided, bool)
        or not isinstance(disable_depth_test, bool)
    ):
        return None
    uses_distortion = state.get("usesDistortion")
    require(
        uses_distortion is None or isinstance(uses_distortion, bool),
        "source renderState.usesDistortion must be boolean or null",
    )
    return {
        "blendMode": blend_mode,
        "twoSided": two_sided,
        "depthTestEnabled": not disable_depth_test,
        # The committed source contract captures bDisableDepthTest, not the
        # depth-write policy.  Masked rows must remain explicitly unresolved.
        "depthWrite": None,
        "depthWriteEvidence": "NOT_CAPTURED",
        "usesDistortion": uses_distortion,
    }


def _current_render_state(render_profile: str) -> dict[str, Any]:
    if render_profile.startswith("additive_"):
        blend_mode = "BLEND_Additive"
    elif render_profile.startswith("alpha_"):
        blend_mode = "BLEND_Translucent"
    elif render_profile.startswith("opaque_"):
        blend_mode = "BLEND_Opaque"
    else:
        raise ApplicationError(f"unsupported current renderProfile: {render_profile}")
    require(
        "_one_sided_" in render_profile
        or "_two_sided_" in render_profile
        or "_back_" in render_profile,
        f"renderProfile has no sidedness: {render_profile}",
    )
    require(
        render_profile.endswith("depth_read")
        or render_profile.endswith("depth_write")
        or render_profile.endswith("depth_disabled"),
        f"renderProfile has no depth policy: {render_profile}",
    )
    return {
        "renderProfile": render_profile,
        "blendMode": blend_mode,
        "twoSided": "_two_sided_" in render_profile,
        "depthTestEnabled": not render_profile.endswith("depth_disabled"),
        "depthWrite": render_profile.endswith("depth_write"),
    }


def _render_state_comparison(
    source: Mapping[str, Any] | None,
    current: Mapping[str, Any],
) -> dict[str, Any]:
    if source is None:
        return {
            "status": "SOURCE_STATE_UNRESOLVED",
            "mismatchAxes": [],
            "depthWriteCompared": False,
        }
    mismatch_axes: list[str] = []
    if source["blendMode"] != current["blendMode"]:
        mismatch_axes.append("blendMode")
    if source["twoSided"] != current["twoSided"]:
        mismatch_axes.append("twoSided")
    if source["depthTestEnabled"] != current["depthTestEnabled"]:
        mismatch_axes.append("depthTest")
    return {
        "status": (
            "MISMATCH_AVAILABLE_AXES"
            if mismatch_axes
            else "MATCH_AVAILABLE_AXES"
        ),
        "mismatchAxes": mismatch_axes,
        "depthWriteCompared": False,
    }


def _source_identity_projection(
    occurrence: Mapping[str, Any],
    identity: Mapping[str, Any] | None,
) -> dict[str, Any]:
    source_material_path = occurrence.get("sourceMaterialPath")
    inventory_parent = occurrence.get("sourceParentMaterialPath")
    if identity is None:
        return {
            "sourceMaterialPath": source_material_path,
            "inventoryEffectiveParentMaterialPath": inventory_parent,
            "contractJoinStatus": "NO_COMMITTED_SOURCE_IDENTITY",
            "contractParentMaterialPath": None,
            "parentAgreement": None,
            "profileId": None,
            "runtimeShaderProfileId": None,
            "productAdmissionStatus": None,
            "semanticStatus": None,
            "sourceEvidenceResolved": None,
        }
    require(
        identity.get("sourceMaterialPath") == source_material_path,
        "source contract join changed sourceMaterialPath identity",
    )
    contract_parent = identity.get("parentMaterialPath")
    return {
        "sourceMaterialPath": source_material_path,
        "inventoryEffectiveParentMaterialPath": inventory_parent,
        "contractJoinStatus": "EXACT_SOURCE_MATERIAL_IDENTITY",
        "contractParentMaterialPath": contract_parent,
        "parentAgreement": contract_parent == inventory_parent,
        "profileId": identity.get("profileId"),
        "runtimeShaderProfileId": identity.get("runtimeShaderProfileId"),
        "productAdmissionStatus": identity.get("productAdmissionStatus"),
        "semanticStatus": identity.get("semanticStatus"),
        "sourceEvidenceResolved": identity.get("sourceEvidenceResolved"),
    }


def _is_simple_rt0_packet_pending(
    occurrence: Mapping[str, Any],
    source_state: Mapping[str, Any] | None,
) -> bool:
    if source_state is None:
        return False
    if occurrence["program"]["status"] != "DXBC_OCCURRENCE_EXACT":
        return False
    if occurrence["layout"]["status"] not in {
        "NAMED_NATIVE_WIRE_ONLY_WITHIN_COUNT_CAPS",
        "EXACT_VARIANT_NATIVE_WIRE_ONLY_REQUIRES_PACKET_TRANSLATION",
    }:
        return False
    if occurrence["descriptor"]["status"] != "SOURCE_VALUES_PRESENT_UNPACKED":
        return False
    if source_state["blendMode"] not in {"BLEND_Additive", "BLEND_Translucent"}:
        return False
    if source_state["depthTestEnabled"] is not True:
        return False
    return not SIMPLE_RT0_ADVANCED_BLOCKERS.intersection(occurrence["blockers"])


def _application_state(
    occurrence: Mapping[str, Any],
    source_identity: Mapping[str, Any] | None,
    source_state: Mapping[str, Any] | None,
    binding: Mapping[str, Any] | None,
) -> str:
    fine_kind = occurrence["fineRendererKind"]
    if fine_kind in FEATURE_DEFERRED_FINE_KIND_SET:
        return "FEATURE_DEFERRED"
    require(
        fine_kind in HORIZONTAL_FINE_KIND_SET,
        f"unexpected four-character Product fine renderer kind: {fine_kind}",
    )
    if binding is not None:
        require(
            occurrence["program"]["status"] == "TYPED_RUNTIME_PROGRAM_DECLARED"
            and occurrence["layout"]["status"] == "TYPED_PACKET_CLOSED"
            and occurrence["descriptor"]["status"] == "TYPED_VALUES_CLOSED",
            "current registry binding is not backed by a closed inline packet",
        )
        return "CURRENT_BOUND_INLINE_EXACT"
    if (
        occurrence["program"]["status"] == "TYPED_RUNTIME_PROGRAM_DECLARED"
        and occurrence["layout"]["status"] == "TYPED_PACKET_CLOSED"
        and occurrence["descriptor"]["status"] == "TYPED_VALUES_CLOSED"
    ):
        return "INLINE_MIRROR_CANDIDATE"
    if _is_simple_rt0_packet_pending(occurrence, source_state):
        return "SOURCE_EXACT_SIMPLE_RT0_PACKET_PENDING"
    if occurrence["program"]["status"] == "DXBC_OCCURRENCE_EXACT":
        return "SOURCE_EXACT_PACKET_PENDING"
    if (
        occurrence["program"]["status"]
        in {"DXBC_FAMILY_REPRESENTATIVE_ONLY", "BOUNDED_SOURCE_PROFILE_ONLY"}
        and source_identity is not None
        and source_identity.get("productAdmissionStatus")
        == "ADMITTED_RECONSTRUCTED_PROFILE"
        and occurrence["descriptor"]["status"]
        == "SOURCE_VALUES_PRESENT_UNPACKED"
        and source_state is not None
    ):
        return "PROJECT_RECONSTRUCTION_PENDING"
    return "EVIDENCE_BLOCKED"


def _binding_identity(binding: Mapping[str, Any] | None) -> dict[str, Any] | None:
    if binding is None:
        return None
    projected = {
        "effectAssetId": binding["effectAssetId"],
        "elementId": binding["elementId"],
        "programId": binding["programId"],
        "layoutId": binding["layoutId"],
        "descriptorId": binding["descriptorId"],
        "adapterId": binding["adapterId"],
    }
    projected["bindingSha256"] = canonical_sha256(projected)
    return projected


def _derived_blockers(
    occurrence: Mapping[str, Any],
    source_projection: Mapping[str, Any],
    source_state: Mapping[str, Any] | None,
) -> list[str]:
    blockers = set(occurrence["blockers"])
    if source_projection["contractJoinStatus"] == "NO_COMMITTED_SOURCE_IDENTITY":
        blockers.add("SOURCE_MATERIAL_CONTRACT_IDENTITY_MISSING")
    elif source_projection["parentAgreement"] is False:
        blockers.add("SOURCE_PARENT_IDENTITY_DIVERGENT")
    if source_state is None:
        blockers.add("SOURCE_RENDER_STATE_UNRESOLVED")
    else:
        blockers.add("SOURCE_DEPTH_WRITE_NOT_CAPTURED")
        if source_state["blendMode"] == "BLEND_Masked":
            blockers.add("MASKED_DEPTH_WRITE_UNRESOLVED")
    if occurrence["fineRendererKind"] == "LEGACY_STANDALONE_SPRITE":
        blockers.add("FEATURE_DEFERRED_LEGACY_STANDALONE_SPRITE")
    elif occurrence["fineRendererKind"] in {
        "AUTHORED_LEGACY_TRAIL",
        "CASCADE_RIBBON",
    }:
        blockers.add("FEATURE_DEFERRED_RIBBON")
    return sorted(blockers)


def _row_sort_key(row: Mapping[str, Any]) -> tuple[Any, ...]:
    return (
        row["domain"],
        row["effectAssetId"],
        row["elementOrder"],
        row["elementId"],
        row["ledgerOccurrenceId"],
    )


def _state_category(state: Mapping[str, Any] | None) -> str:
    if state is None:
        return "UNRESOLVED"
    return ".".join(
        (
            state["blendMode"],
            "two-sided" if state["twoSided"] else "one-sided",
            "depth-test" if state["depthTestEnabled"] else "depth-disabled",
            "depth-write-unresolved",
        )
    )


def _counter(values: Any) -> dict[str, int]:
    return dict(sorted(Counter(values).items()))


def _summary(rows: list[dict[str, Any]], registry_binding_count: int) -> dict[str, Any]:
    bound_rows = [row for row in rows if row["bindingIdentity"] is not None]
    source_resolved = [
        row for row in rows if row["renderState"]["source"] is not None
    ]
    mismatch_axes = Counter(
        axis
        for row in rows
        for axis in row["renderState"]["comparison"]["mismatchAxes"]
    )
    fine_kind_counts = _counter(row["fineRendererKind"] for row in rows)
    horizontal_count = sum(
        count
        for kind, count in fine_kind_counts.items()
        if kind in HORIZONTAL_FINE_KIND_SET
    )
    deferred_count = sum(
        count
        for kind, count in fine_kind_counts.items()
        if kind in FEATURE_DEFERRED_FINE_KIND_SET
    )
    return {
        "productOccurrenceCount": len(rows),
        "productAssetCount": len({row["effectAssetId"] for row in rows}),
        "horizontalFineOccurrenceCount": horizontal_count,
        "featureDeferredOccurrenceCount": deferred_count,
        "domainOccurrenceCounts": _counter(row["domain"] for row in rows),
        "fineRendererKindCounts": fine_kind_counts,
        "coarseCarrierCounts": _counter(row["coarseCarrier"] for row in rows),
        "applicationStateCounts": _counter(row["applicationState"] for row in rows),
        "programStatusCounts": _counter(row["axes"]["program"]["status"] for row in rows),
        "layoutStatusCounts": _counter(row["axes"]["layout"]["status"] for row in rows),
        "descriptorStatusCounts": _counter(row["axes"]["descriptor"]["status"] for row in rows),
        "adapterStatusCounts": _counter(row["axes"]["adapter"]["status"] for row in rows),
        "sourceIdentityJoinStatusCounts": _counter(
            row["sourceMaterial"]["contractJoinStatus"] for row in rows
        ),
        "sourceParentAgreementCounts": _counter(
            "NOT_JOINED"
            if row["sourceMaterial"]["parentAgreement"] is None
            else (
                "MATCH"
                if row["sourceMaterial"]["parentAgreement"]
                else "DIVERGENT"
            )
            for row in rows
        ),
        "sourceRenderStateResolvedCount": len(source_resolved),
        "sourceRenderStateUnresolvedCount": len(rows) - len(source_resolved),
        "sourceRenderStateCategoryCounts": _counter(
            _state_category(row["renderState"]["source"]) for row in rows
        ),
        "currentRenderProfileCounts": _counter(
            row["renderState"]["current"]["renderProfile"] for row in rows
        ),
        "renderStateComparisonStatusCounts": _counter(
            row["renderState"]["comparison"]["status"] for row in rows
        ),
        "renderStateMismatchAxisCounts": dict(
            sorted(mismatch_axes.items())
        ),
        "registryBindingCount": registry_binding_count,
        "targetRegistryBindingCount": len(bound_rows),
        "unboundOccurrenceCount": len(rows) - len(bound_rows),
        "manualReviewCounts": _counter(row["manualReview"] for row in rows),
        "duplicateOccurrenceKeyCount": 0,
        "duplicateRegistryBindingKeyCount": 0,
        "registryReverseJoinMissingCount": 0,
    }


def _validate_source_contract(source_contract: Mapping[str, Any]) -> dict[str, dict[str, Any]]:
    require(
        source_contract.get("schema") == "lostark.effect-source-material-contract",
        "unexpected FourClassCombat source contract schema",
    )
    require(source_contract.get("formatVersion") == 1,
            "unexpected FourClassCombat source contract version")
    require(
        source_contract.get("effectAssetId")
        == "effect.four-class.combat.source-material-contract",
        "unexpected FourClassCombat source contract identity",
    )
    identities = source_contract.get("materialIdentities")
    require(isinstance(identities, list), "source contract materialIdentities must be an array")
    by_path: dict[str, dict[str, Any]] = {}
    for index, identity in enumerate(identities):
        require(isinstance(identity, dict), f"source identity {index} must be an object")
        source_path = identity.get("sourceMaterialPath")
        require(isinstance(source_path, str) and source_path, f"source identity {index} has no path")
        require(source_path not in by_path, f"duplicate sourceMaterialPath: {source_path}")
        by_path[source_path] = identity
    return by_path


def _build_validated_material_registry() -> dict[str, Any]:
    try:
        return material_registry.build_registry(
            REGISTRY_PATH,
            EFFECT_CATALOG_PATH,
            DATA_ROOT,
        )
    except (material_registry.ContractError, OSError) as exc:
        raise ApplicationError(
            f"material-program registry validation failed: {exc}"
        ) from exc


def build_application(repository_root: Path = ROOT) -> dict[str, Any]:
    repository_root = repository_root.resolve()
    require(repository_root == ROOT.resolve(), "alternate repository roots are not supported")

    inventory = tuple_inventory.build_inventory(repository_root)
    inventory_without_hash = dict(inventory)
    inventory_hash = inventory_without_hash.pop("artifactSha256", None)
    require(
        isinstance(inventory_hash, str)
        and inventory_hash == canonical_sha256(inventory_without_hash),
        "in-memory tuple inventory artifact hash is invalid",
    )

    source_contract = load_json(SOURCE_CONTRACT_PATH, "FourClassCombat source contract")
    source_by_path = _validate_source_contract(source_contract)

    registry = _build_validated_material_registry()
    validated_registry_sha256 = canonical_sha256(registry)

    registry_bindings = registry["bindings"]
    bindings_by_key: dict[tuple[str, str], dict[str, Any]] = {}
    for binding in registry_bindings:
        key = (binding["effectAssetId"], binding["elementId"])
        require(key not in bindings_by_key, f"duplicate registry binding key: {key[0]}/{key[1]}")
        bindings_by_key[key] = binding

    adapters_by_id = {
        row["adapterCandidateId"]: row
        for row in inventory["adapterCandidates"]
    }
    product_occurrences = [
        row
        for row in inventory["occurrences"]
        if row["domain"] in TARGET_DOMAIN_SET
        and row["scopeBits"]["productConsumed"] is True
    ]
    require(
        len(product_occurrences) == EXPECTED_PRODUCT_OCCURRENCE_COUNT,
        "four-character Product occurrence denominator changed",
    )

    rows: list[dict[str, Any]] = []
    occurrence_keys: set[tuple[str, str]] = set()
    for occurrence in product_occurrences:
        key = (occurrence["effectAssetId"], occurrence["elementId"])
        require(key not in occurrence_keys, f"duplicate Product occurrence: {key[0]}/{key[1]}")
        occurrence_keys.add(key)

        source_path = occurrence.get("sourceMaterialPath")
        source_identity = (
            source_by_path.get(source_path)
            if isinstance(source_path, str) and source_path
            else None
        )
        source_projection = _source_identity_projection(occurrence, source_identity)
        source_state = _exact_source_render_state(source_identity)

        adapter_axis = occurrence["adapter"]
        adapter_id = adapter_axis.get("adapterCandidateId")
        require(isinstance(adapter_id, str), f"Product occurrence has no Adapter candidate: {key}")
        adapter_candidate = adapters_by_id.get(adapter_id)
        require(adapter_candidate is not None, f"unknown Adapter candidate: {adapter_id}")
        render_profile = adapter_candidate.get("renderProfile")
        require(isinstance(render_profile, str), f"Adapter has no renderProfile: {adapter_id}")
        current_state = _current_render_state(render_profile)
        comparison = _render_state_comparison(source_state, current_state)

        binding = bindings_by_key.get(key)
        application_state = _application_state(
            occurrence,
            source_identity,
            source_state,
            binding,
        )
        row: dict[str, Any] = {
            "ledgerOccurrenceId": occurrence["occurrenceId"],
            "effectAssetId": occurrence["effectAssetId"],
            "elementId": occurrence["elementId"],
            "elementOrder": occurrence["elementOrder"],
            "domain": occurrence["domain"],
            "fineRendererKind": occurrence["fineRendererKind"],
            "coarseCarrier": occurrence["carrier"],
            "horizontalScopeStatus": (
                "IN_SCOPE"
                if occurrence["fineRendererKind"] in HORIZONTAL_FINE_KIND_SET
                else "FEATURE_DEFERRED"
            ),
            "applicationState": application_state,
            "runtimeProofStatus": (
                "REGISTRY_BINDING_INLINE_PACKET_EXACT"
                if binding is not None
                else "NOT_PROVEN"
            ),
            "bindingIdentity": _binding_identity(binding),
            "sourceMaterial": source_projection,
            "renderState": {
                "source": source_state,
                "current": current_state,
                "comparison": comparison,
            },
            "axes": {
                "program": copy.deepcopy(occurrence["program"]),
                "layout": copy.deepcopy(occurrence["layout"]),
                "descriptor": copy.deepcopy(occurrence["descriptor"]),
                "adapter": copy.deepcopy(occurrence["adapter"]),
            },
            "blockers": _derived_blockers(
                occurrence,
                source_projection,
                source_state,
            ),
            "manualReview": MANUAL_REVIEW,
        }
        row["rowSha256"] = canonical_sha256(row)
        rows.append(row)

    rows.sort(key=_row_sort_key)

    target_binding_keys = {
        key
        for key in bindings_by_key
        if key[0].startswith(
            (
                "effect.artist.",
                "effect.dimensionmaster.",
                "effect.lancemaster.",
                "effect.warlord.",
            )
        )
    }
    missing_reverse_joins = sorted(target_binding_keys - occurrence_keys)
    require(
        not missing_reverse_joins,
        "four-character registry binding does not reverse-join Product: "
        + "/".join(missing_reverse_joins[0])
        if missing_reverse_joins
        else "",
    )

    summary = _summary(rows, len(registry_bindings))
    require(summary["horizontalFineOccurrenceCount"] == EXPECTED_HORIZONTAL_FINE_OCCURRENCE_COUNT,
            "horizontal fine occurrence denominator changed")
    require(summary["featureDeferredOccurrenceCount"] == EXPECTED_FEATURE_DEFERRED_OCCURRENCE_COUNT,
            "feature-deferred occurrence denominator changed")
    require(summary["fineRendererKindCounts"] == EXPECTED_FINE_KIND_COUNTS,
            "four-character fine renderer taxonomy changed")

    occurrence_projection = [
        {
            "ledgerOccurrenceId": row["ledgerOccurrenceId"],
            "effectAssetId": row["effectAssetId"],
            "elementId": row["elementId"],
        }
        for row in rows
    ]
    source_projection = [
        {
            "ledgerOccurrenceId": row["ledgerOccurrenceId"],
            "sourceMaterialPath": row["sourceMaterial"]["sourceMaterialPath"],
            "contractJoinStatus": row["sourceMaterial"]["contractJoinStatus"],
            "contractParentMaterialPath": row["sourceMaterial"]["contractParentMaterialPath"],
        }
        for row in rows
    ]
    binding_projection = [
        {
            "ledgerOccurrenceId": row["ledgerOccurrenceId"],
            "bindingIdentity": row["bindingIdentity"],
        }
        for row in rows
        if row["bindingIdentity"] is not None
    ]
    state_projection = [
        {
            "ledgerOccurrenceId": row["ledgerOccurrenceId"],
            "applicationState": row["applicationState"],
        }
        for row in rows
    ]

    document: dict[str, Any] = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "identity": {
            "ledgerId": LEDGER_ID,
            "generation": 1,
            "revision": 1,
            "scope": "FOUR_CHARACTER_PRODUCT_OCCURRENCES",
        },
        "inputs": {
            "tupleInventory": {
                "builder": "Tools/EffectPipeline/build_effect_tuple_cohort_inventory.py:build_inventory",
                "mode": "IN_MEMORY_CURRENT_HEAD",
                "checkedInArtifactUsed": False,
                "artifactSha256": inventory_hash,
            },
            "sourceMaterialContract": {
                "path": relative_path(SOURCE_CONTRACT_PATH),
                "fileSha256": file_sha256(SOURCE_CONTRACT_PATH),
                "joinMode": "EXACT_SOURCE_MATERIAL_PATH_ONLY",
                "materialIdentityCount": len(source_by_path),
            },
            "materialProgramRegistry": {
                "path": relative_path(REGISTRY_PATH),
                "fileSha256": file_sha256(REGISTRY_PATH),
                "validatedRegistrySha256": validated_registry_sha256,
                "bindingCount": len(registry_bindings),
                "bindingPacketValidation": "INLINE_BIT_EXACT",
            },
        },
        "policy": {
            "trackBTupleInventoryRuntimeProofAllowed": False,
            "trackBArtifactPromotedToRuntimeProof": False,
            "bindingIdentityRequiredForCurrentBound": True,
            "bindingAbsentIdentity": None,
            "sourceJoinMode": "EXACT_SOURCE_MATERIAL_PATH_ONLY",
            "sourceDepthWriteEvidence": "NOT_CAPTURED",
            "maskedDepthWriteAdmission": "BLOCKED_UNTIL_EVIDENCE",
            "automaticBindingMutation": False,
            "manualReviewDefault": MANUAL_REVIEW,
        },
        "summary": summary,
        "projections": {
            "occurrenceKeyProjectionSha256": canonical_sha256(occurrence_projection),
            "sourceIdentityJoinProjectionSha256": canonical_sha256(source_projection),
            "bindingReverseJoinProjectionSha256": canonical_sha256(binding_projection),
            "applicationStateProjectionSha256": canonical_sha256(state_projection),
        },
        "occurrences": rows,
        "transaction": {
            "model": "PARSE_VALIDATE_STAGE_ATOMIC_REPLACE",
            "partialCommitAllowed": False,
        },
    }
    document["artifactSha256"] = canonical_sha256(document)
    validate_application(document, validated_registry_sha256)
    return document


def validate_application(
    document: Mapping[str, Any],
    expected_validated_registry_sha256: str | None = None,
) -> None:
    required_root_keys = {
        "schema",
        "formatVersion",
        "identity",
        "inputs",
        "policy",
        "summary",
        "projections",
        "occurrences",
        "transaction",
        "artifactSha256",
    }
    require(set(document) == required_root_keys, "application ledger root keys changed")
    require(document["schema"] == SCHEMA, "application ledger schema changed")
    require(document["formatVersion"] == FORMAT_VERSION, "application ledger version changed")
    require(document["policy"]["trackBTupleInventoryRuntimeProofAllowed"] is False,
            "Track B was promoted to runtime proof")
    require(document["policy"]["trackBArtifactPromotedToRuntimeProof"] is False,
            "Track B artifact was promoted to runtime proof")
    require(document["policy"]["sourceDepthWriteEvidence"] == "NOT_CAPTURED",
            "source depth-write evidence was invented")
    if expected_validated_registry_sha256 is None:
        expected_validated_registry_sha256 = canonical_sha256(
            _build_validated_material_registry()
        )
    recorded_validated_registry_sha256 = document["inputs"][
        "materialProgramRegistry"
    ].get("validatedRegistrySha256")
    require(
        recorded_validated_registry_sha256
        == expected_validated_registry_sha256,
        "fully validated material-program registry identity is stale",
    )

    rows = document["occurrences"]
    require(isinstance(rows, list), "occurrences must be an array")
    require(len(rows) == EXPECTED_PRODUCT_OCCURRENCE_COUNT,
            "application ledger occurrence denominator changed")
    require(rows == sorted(rows, key=_row_sort_key), "application rows are not deterministic")

    occurrence_ids: set[str] = set()
    occurrence_keys: set[tuple[str, str]] = set()
    binding_keys: set[tuple[str, str]] = set()
    for index, row in enumerate(rows):
        require(isinstance(row, dict), f"occurrence row {index} must be an object")
        occurrence_id = row.get("ledgerOccurrenceId")
        require(isinstance(occurrence_id, str) and occurrence_id,
                f"occurrence row {index} has no identity")
        require(occurrence_id not in occurrence_ids, f"duplicate ledger occurrenceId: {occurrence_id}")
        occurrence_ids.add(occurrence_id)
        key = (row.get("effectAssetId"), row.get("elementId"))
        require(all(isinstance(value, str) and value for value in key),
                f"occurrence row {index} has an invalid Product key")
        require(key not in occurrence_keys, f"duplicate Product key: {key[0]}/{key[1]}")
        occurrence_keys.add(key)
        require(row.get("domain") in TARGET_DOMAIN_SET, f"row {index} has an invalid domain")
        fine_kind = row.get("fineRendererKind")
        require(fine_kind in HORIZONTAL_FINE_KIND_SET | FEATURE_DEFERRED_FINE_KIND_SET,
                f"row {index} has an unexpected fine renderer kind")
        expected_scope = "IN_SCOPE" if fine_kind in HORIZONTAL_FINE_KIND_SET else "FEATURE_DEFERRED"
        require(row.get("horizontalScopeStatus") == expected_scope,
                f"row {index} horizontal scope is inconsistent")
        require(row.get("applicationState") in APPLICATION_STATES,
                f"row {index} application state is invalid")
        if fine_kind in FEATURE_DEFERRED_FINE_KIND_SET:
            require(row["applicationState"] == "FEATURE_DEFERRED",
                    f"row {index} deferred feature was admitted")
        require(row.get("manualReview") == MANUAL_REVIEW,
                f"row {index} manual review was pre-approved")

        binding = row.get("bindingIdentity")
        if binding is None:
            require(row.get("applicationState") != "CURRENT_BOUND_INLINE_EXACT",
                    f"row {index} claims current bound state without a Binding")
            require(row.get("runtimeProofStatus") == "NOT_PROVEN",
                    f"row {index} has runtime proof without a Binding")
        else:
            require(isinstance(binding, dict), f"row {index} binding identity is malformed")
            require(row["applicationState"] == "CURRENT_BOUND_INLINE_EXACT",
                    f"row {index} bound state is inconsistent")
            require(row.get("runtimeProofStatus") == "REGISTRY_BINDING_INLINE_PACKET_EXACT",
                    f"row {index} bound proof status is inconsistent")
            require((binding["effectAssetId"], binding["elementId"]) == key,
                    f"row {index} binding does not reverse-join")
            require(key not in binding_keys, f"duplicate binding reverse join: {key}")
            binding_keys.add(key)
            binding_without_hash = dict(binding)
            binding_hash = binding_without_hash.pop("bindingSha256", None)
            require(binding_hash == canonical_sha256(binding_without_hash),
                    f"row {index} binding hash is invalid")

        render_state = row.get("renderState")
        require(isinstance(render_state, dict), f"row {index} renderState is malformed")
        source_state = render_state.get("source")
        current_state = render_state.get("current")
        comparison = render_state.get("comparison")
        require(isinstance(current_state, dict), f"row {index} current renderState is malformed")
        require(isinstance(comparison, dict), f"row {index} comparison is malformed")
        require(comparison.get("depthWriteCompared") is False,
                f"row {index} invented a source depth-write comparison")
        mismatch_axes = comparison.get("mismatchAxes")
        require(isinstance(mismatch_axes, list), f"row {index} mismatch axes are malformed")
        require(mismatch_axes == [axis for axis in MISMATCH_AXIS_ORDER if axis in mismatch_axes],
                f"row {index} mismatch axes are not ordered or unique")
        if source_state is not None:
            require(source_state.get("depthWrite") is None,
                    f"row {index} invented source depth-write state")
            require(source_state.get("depthWriteEvidence") == "NOT_CAPTURED",
                    f"row {index} source depth-write evidence changed")
            if source_state.get("blendMode") == "BLEND_Masked":
                require("MASKED_DEPTH_WRITE_UNRESOLVED" in row["blockers"],
                        f"row {index} admitted masked depth-write without evidence")
        require(
            comparison == _render_state_comparison(source_state, current_state),
            f"row {index} render-state comparison is stale",
        )

        row_without_hash = dict(row)
        row_hash = row_without_hash.pop("rowSha256", None)
        require(row_hash == canonical_sha256(row_without_hash),
                f"row {index} hash is invalid")

    summary = document["summary"]
    require(summary == _summary(rows, summary["registryBindingCount"]),
            "application ledger summary is stale")
    require(summary["fineRendererKindCounts"] == EXPECTED_FINE_KIND_COUNTS,
            "application ledger fine renderer counts changed")
    require(summary["horizontalFineOccurrenceCount"] == EXPECTED_HORIZONTAL_FINE_OCCURRENCE_COUNT,
            "horizontal fine denominator changed")
    require(summary["featureDeferredOccurrenceCount"] == EXPECTED_FEATURE_DEFERRED_OCCURRENCE_COUNT,
            "feature-deferred denominator changed")
    require(summary["targetRegistryBindingCount"] == len(binding_keys),
            "target Binding count differs from reverse joins")
    require(
        summary["registryBindingCount"]
        == document["inputs"]["materialProgramRegistry"]["bindingCount"],
        "registry Binding count differs from its input identity",
    )

    occurrence_projection = [
        {
            "ledgerOccurrenceId": row["ledgerOccurrenceId"],
            "effectAssetId": row["effectAssetId"],
            "elementId": row["elementId"],
        }
        for row in rows
    ]
    source_projection = [
        {
            "ledgerOccurrenceId": row["ledgerOccurrenceId"],
            "sourceMaterialPath": row["sourceMaterial"]["sourceMaterialPath"],
            "contractJoinStatus": row["sourceMaterial"]["contractJoinStatus"],
            "contractParentMaterialPath": row["sourceMaterial"]["contractParentMaterialPath"],
        }
        for row in rows
    ]
    binding_projection = [
        {
            "ledgerOccurrenceId": row["ledgerOccurrenceId"],
            "bindingIdentity": row["bindingIdentity"],
        }
        for row in rows
        if row["bindingIdentity"] is not None
    ]
    state_projection = [
        {
            "ledgerOccurrenceId": row["ledgerOccurrenceId"],
            "applicationState": row["applicationState"],
        }
        for row in rows
    ]
    require(
        document["projections"]
        == {
            "occurrenceKeyProjectionSha256": canonical_sha256(occurrence_projection),
            "sourceIdentityJoinProjectionSha256": canonical_sha256(source_projection),
            "bindingReverseJoinProjectionSha256": canonical_sha256(binding_projection),
            "applicationStateProjectionSha256": canonical_sha256(state_projection),
        },
        "application ledger projections are stale",
    )
    document_without_hash = dict(document)
    artifact_hash = document_without_hash.pop("artifactSha256", None)
    require(artifact_hash == canonical_sha256(document_without_hash),
            "application ledger artifact hash is invalid")


def write_atomic(path: Path, payload: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary_path: Path | None = None
    try:
        descriptor, temporary_name = tempfile.mkstemp(
            prefix=path.name + ".",
            suffix=".tmp",
            dir=path.parent,
        )
        temporary_path = Path(temporary_name)
        with os.fdopen(descriptor, "wb") as stream:
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary_path, path)
        temporary_path = None
    finally:
        if temporary_path is not None and temporary_path.exists():
            temporary_path.unlink()


def run(
    output_path: Path,
    check: bool,
    document: dict[str, Any] | None = None,
) -> int:
    application = document if document is not None else build_application()
    validate_application(application)
    payload = pretty_json_bytes(application)
    if check:
        if not output_path.is_file() or output_path.read_bytes() != payload:
            print(f"STALE: {output_path}", file=sys.stderr)
            return 1
        print(f"PASS: {output_path}")
        return 0
    write_atomic(output_path, payload)
    print(
        f"WROTE: {output_path} "
        f"({application['summary']['productOccurrenceCount']} Product occurrences)"
    )
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--output", type=Path, default=OUTPUT_PATH)
    args = parser.parse_args(argv)
    try:
        return run(args.output.resolve(), args.check)
    except (ApplicationError, OSError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
