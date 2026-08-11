#!/usr/bin/env python3
"""Independent approval pins for Artist 31470 reconstructed render resources.

The builder derives a reviewable policy candidate from the frozen reconstructed
runtime program.  This module owns the independent human approval boundary.  A
caller therefore cannot change a renderer/material join or a D3D descriptor,
re-seal all row/root digests, and have the approved validator accept it.

Nothing in this module promotes the decisions to source evidence or Product
runtime admission.
"""

from __future__ import annotations

import copy
import hashlib
import json
from typing import Any


# The only renderer rows for which occurrence + exact runtime asset identity
# produces two Material-input candidates.  These choices are explicit approval
# decisions, never a runtime basename/role heuristic.
APPROVED_AMBIGUOUS_RENDERER_BINDINGS = {
    (
        "fx_pc_sdm_07.par_v_smd_onestroke_swing_01::action-31470/stage-000/"
        "notify-018::FX_PC_SDM_07.par_v_smd_onestroke_swing_01."
        "particlespriteemitter_15::renderer-texture:base"
    ): "material-input-e51237e20a813da8",
    (
        "fx_pc_sdm_07.par_v_sdm_onestroke_hit_01::action-31470/stage-000/"
        "notify-022::FX_PC_SDM_07.par_v_sdm_onestroke_hit_01."
        "particlespriteemitter_16::renderer-texture:base"
    ): "material-input-787a89b9e8277bec",
    (
        "fx_pc_sdm_07.par_v_sdm_onestroke_hit_01::action-31470/stage-000/"
        "notify-022::FX_PC_SDM_07.par_v_sdm_onestroke_hit_01."
        "particlespriteemitter_10::renderer-texture:base"
    ): "material-input-7aed8cfe5ba9669b",
}


# Frozen only after the generated receipt and all mutation regressions pass.
APPROVED_DECISION_PROJECTION_SHA256 = (
    "4731ed9c2882c948373ec54f56087803145447851f3fc793fb8e9fa9d96cc957"
)
APPROVED_RECEIPT_PROJECTION_SHA256 = (
    "d643c9bf1bc2f10a887c805534b28e4322646cea426656de61b894e5b6284644"
)


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(
        json.dumps(
            value,
            ensure_ascii=False,
            sort_keys=True,
            separators=(",", ":"),
            allow_nan=False,
        ).encode("utf-8")
    ).hexdigest()


def approved_decision_projection(receipt: dict[str, Any]) -> dict[str, Any]:
    """Project only the policy decisions that an independent reviewer freezes."""

    return {
        "approvalId": receipt["approvalId"],
        "approvalContract": copy.deepcopy(receipt["approvalContract"]),
        "neutralProviders": copy.deepcopy(receipt["neutralProviders"]),
        "recipeTextureBindings": copy.deepcopy(receipt["recipeTextureBindings"]),
        "rendererSlotBindings": copy.deepcopy(receipt["rendererSlotBindings"]),
        "renderStateDescriptors": copy.deepcopy(receipt["renderStateDescriptors"]),
        "blockerProjection": copy.deepcopy(receipt["blockerProjection"]),
        "admission": copy.deepcopy(receipt["admission"]),
    }


def decision_projection_sha256(receipt: dict[str, Any]) -> str:
    return canonical_sha256(approved_decision_projection(receipt))


def approved_receipt_projection(receipt: dict[str, Any]) -> dict[str, Any]:
    candidate = copy.deepcopy(receipt)
    candidate.pop("receiptSha256", None)
    return candidate


def receipt_projection_sha256(receipt: dict[str, Any]) -> str:
    return canonical_sha256(approved_receipt_projection(receipt))


def require_approved_receipt(receipt: dict[str, Any]) -> None:
    decision_actual = decision_projection_sha256(receipt)
    if decision_actual != APPROVED_DECISION_PROJECTION_SHA256:
        raise ValueError(
            "Material render-resource decisions do not match independent approval pin: "
            f"expected={APPROVED_DECISION_PROJECTION_SHA256} actual={decision_actual}"
        )
    receipt_actual = receipt_projection_sha256(receipt)
    if receipt_actual != APPROVED_RECEIPT_PROJECTION_SHA256:
        raise ValueError(
            "Material render-resource receipt does not match independent approval pin: "
            f"expected={APPROVED_RECEIPT_PROJECTION_SHA256} actual={receipt_actual}"
        )
