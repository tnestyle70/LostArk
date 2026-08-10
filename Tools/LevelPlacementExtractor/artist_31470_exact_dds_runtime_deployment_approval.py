"""Independent pins for the Artist 31470 exact-DDS runtime deployment."""

from __future__ import annotations

import copy
import hashlib
import json
from typing import Any


AUTHORITY_COMMIT = "fda3b5637847f9205915ad25ff02215424024b88"
AUTHORITY_TREE = "2f00f00851ee93f498dd6c13d6a3055209d4d8c3"

BINDING_RELATIVE_PATH = (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.material-texture-runtime-binding.receipt.json"
)
BINDING_GIT_BLOB = "3105c22a3c8e9b73b47b721ffab72d1254fc1750"
BINDING_TRACKED_TEXT_SHA256 = (
    "3a097a174df6b940989c7ce6c7b4e3b7798256d200cf73e25e694dc827e4346e"
)
BINDING_RECEIPT_SHA256 = (
    "39c91577c09b853fa55a8fd5531c1cddc4fef928d77a6caa7f67c472a56159e0"
)

EXACT_DDS_RELATIVE_PATH = (
    "Data/Effects/Imported/Artist/Materials/skill.31470.exact-dds-recovery.receipt.json"
)
EXACT_DDS_GIT_BLOB = "9e570499765951f1aca21a3364d3e9bcae0f948e"
EXACT_DDS_TRACKED_TEXT_SHA256 = (
    "3b21d1ce5d9fa6575e0d289967584e8198aec42c99aa634929bf01e2b7a97824"
)

DEPLOYMENT_POLICY = "RECONSTRUCTED_RUNTIME_DEPLOYMENT_FROM_EXACT_DDS_FIXTURE_V1"
EXPECTED_RUNTIME_ASSET_IDS = (
    "Effect/Artist/Textures/FX_TEX_00/fx_a_decal_014.dds",
    "Effect/Artist/Textures/FX_TEX_00/fx_a_noise_011.dds",
    "Effect/Artist/Textures/FX_TEX_01/fx_c_atypical_016.dds",
    "Effect/Artist/Textures/FX_TEX_03/fx_e_ring_001_cl.dds",
)
EXPECTED_LOGICAL_TEXTURE_PATHS = (
    "fx_tex_00.fx_a_decal_014",
    "fx_tex_00.fx_a_noise_011",
    "fx_tex_01.fx_c_atypical_016",
    "fx_tex_03.fx_e_ring_001_cl",
)

# The deployment generator is frozen before this projection is calculated.
# targetBefore is intentionally normalized because an idempotent redeploy may
# observe either an absent target or the already-deployed exact bytes.
APPROVED_RECEIPT_PROJECTION_SHA256 = (
    "419f6e2c403b0a39b49e64b2cb46b73ae48a2ed420fa0e355983da73a913d3dc"
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


def approved_receipt_projection(receipt: dict[str, Any]) -> dict[str, Any]:
    candidate = copy.deepcopy(receipt)
    candidate.pop("receiptSha256", None)
    implementation = candidate["sourceEvidence"]["implementationEvidence"]
    implementation["dependencies"] = [
        row
        for row in implementation["dependencies"]
        if row["dependencyId"] != "EXACT_DDS_RUNTIME_DEPLOYMENT_APPROVAL"
    ]
    implementation["dependencyCount"] = len(implementation["dependencies"])
    implementation["projectionSha256"] = canonical_sha256(
        implementation["dependencies"]
    )
    for row in candidate.get("assets", []):
        row["targetBefore"] = {
            "status": "ABSENT_OR_PRESENT_EXACT_EQUAL",
            "byteCount": None,
            "rawSha256": None,
        }
        row_without_digest = copy.deepcopy(row)
        row_without_digest.pop("rowSha256", None)
        row["rowSha256"] = canonical_sha256(row_without_digest)
    candidate["recoveryBackup"] = {
        "anchor": "RUNTIME_RESOURCE_ROOT_PARENT",
        "relativeDirectory": "APPROVED_ABSENT_OR_EXACT_EQUAL_RECOVERY_BACKUP",
        "manifestRelativePath": "APPROVED_RECOVERY_MANIFEST",
        "manifestCanonicalSelfSha256": "0" * 64,
        "manifestRawSha256": "0" * 64,
        "backupPayloadFileCount": -1,
        "absentTargetMarkerCount": -1,
        "preservedAfterCommit": True,
    }
    candidate["summary"]["recoveryBackupPayloadFileCount"] = -1
    candidate["summary"]["recoveryAbsentTargetMarkerCount"] = -1
    return candidate


def receipt_projection_sha256(receipt: dict[str, Any]) -> str:
    return canonical_sha256(approved_receipt_projection(receipt))


def require_approved_receipt(receipt: dict[str, Any]) -> None:
    actual = receipt_projection_sha256(receipt)
    if actual != APPROVED_RECEIPT_PROJECTION_SHA256:
        raise ValueError(
            "exact-DDS runtime deployment does not match independent approval pin: "
            f"expected={APPROVED_RECEIPT_PROJECTION_SHA256} actual={actual}"
        )
