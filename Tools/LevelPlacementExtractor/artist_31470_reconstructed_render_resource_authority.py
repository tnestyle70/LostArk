#!/usr/bin/env python3
"""Independent pins for the Artist 31470 render-resource authority.

The generator derives byte identities and DDS descriptors from the canonical
Resources tree and joins them to an already approved reconstructed Material
policy.  This module is intentionally independent of that derivation.  Its two
projection pins prevent a caller from changing a resource, binding, recipe,
renderer owner, or descriptor and then merely re-sealing the generated JSON.

The pins do not admit Product/runtime execution.  They freeze an offline,
immutable sidecar that a later C++ consumer may stage transactionally.
"""

from __future__ import annotations

import copy
import hashlib
import json
from typing import Any


# Frozen projection pins.  The focused negative matrix independently exercises
# each mutable domain before this lane is eligible for review publication.
APPROVED_DECISION_PROJECTION_SHA256 = (
    "fcef9bb95c5412f1d25f206e207b6eccd8198a26a8994a6ee5ac179498b001de"
)
APPROVED_RECEIPT_PROJECTION_SHA256 = (
    "6f4ed12c7c5b6499ece7cf520436f747e4877a4a89a1584ba57de7324adf8ac4"
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
    """Return every resource/material decision frozen by independent review."""

    return {
        "authorityId": receipt["authorityId"],
        "authorityContract": copy.deepcopy(receipt["authorityContract"]),
        "publisherRuntimeCatalogAuthority": copy.deepcopy(
            receipt["sourceEvidence"]["publisherRuntimeCatalogAuthority"]
        ),
        "textureResources": copy.deepcopy(receipt["textureResources"]),
        "textureBindings": copy.deepcopy(receipt["textureBindings"]),
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
            "render-resource decisions do not match independent pin: "
            f"expected={APPROVED_DECISION_PROJECTION_SHA256} "
            f"actual={decision_actual}"
        )
    receipt_actual = receipt_projection_sha256(receipt)
    if receipt_actual != APPROVED_RECEIPT_PROJECTION_SHA256:
        raise ValueError(
            "render-resource receipt does not match independent pin: "
            f"expected={APPROVED_RECEIPT_PROJECTION_SHA256} "
            f"actual={receipt_actual}"
        )
