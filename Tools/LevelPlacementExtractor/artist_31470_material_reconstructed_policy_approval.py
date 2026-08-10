"""Independent frozen approval pins for Artist F reconstructed Material policy V1.

The builder derives rows from evidence.  This module independently pins the
ordered approval projection and executable oracle artifacts so a coordinated
receipt reseal cannot silently select a different policy.
"""

from __future__ import annotations

import copy
import hashlib
import json
from typing import Any


POLICY_PROJECTION_SHA256 = "1f9383fe927fb0ca3d93c56c9621807d9f48a0d8eb18793519dd91fe01e6c20d"
APPROVED_RECEIPT_PROJECTION_SHA256 = "5b5497020522345363b4cfa220054c87bb1aaf0feaf4993583e048225d35fb4f"
HLSL_TRACKED_TEXT_SHA256 = "2901471f07495ff079c64ea8234ca30cd26300819d56760dbdeabae353c0b718"
HLSL_DXBC_SHA256 = "a7a4192b0d1fa70e19be8a14c8b7001d4e4020b95459bf6f88da9d3b09922bd9"
HLSL_INPUT_BYTES_SHA256 = "751c1969342ccf23e608c403e7d3c126b6bc07febd2d978d767347ecb3aaffd1"
HLSL_OUTPUT_BYTES_SHA256 = "8eb1f9e4a53b681a8c3cf8bdada42718470d278955bc05df97e11922baeb9445"
HLSL_ROW_RESULTS_SHA256 = "b58f66134b824a8f43f02f0bcc2d745aedc7b32d7aa6aab362384e1a9b145d4d"
WARP_DESCRIPTOR_ROW_RESULTS_SHA256 = "5b76e727418c588efd614524e5202b3aaf7040ba4064d0faafe7195ce994c49d"
WARP_SRV_ROW_RESULTS_SHA256 = "47d4c6bea3fa30805ae5a085cfe2094c56766a04ae45d4c76120dbefecedb14f"


def _canonical_sha256(value: Any) -> str:
    payload = json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")
    return hashlib.sha256(payload).hexdigest()


def policy_projection(rows: list[dict[str, Any]]) -> list[dict[str, Any]]:
    # The entire row is approval-relevant: owner/evidence/fidelity/admission
    # fields must not disappear behind a narrower selected-value projection.
    return copy.deepcopy(rows)


def approved_receipt_projection(receipt: dict[str, Any]) -> dict[str, Any]:
    source_evidence = dict(receipt["sourceEvidence"])
    # Avoid a self-hash cycle: the approval module's own text hash is checked by
    # the generated candidate, while every other source/evidence identity is
    # independently pinned here.
    source_evidence.pop("approvalTrackedTextSha256", None)
    closure = copy.deepcopy(source_evidence["directImportClosure"])
    closure["dependencies"] = [
        row
        for row in closure["dependencies"]
        if row["dependencyId"] != "RECONSTRUCTED_POLICY_APPROVAL"
    ]
    closure["dependencyCount"] = len(closure["dependencies"])
    closure["projectionSha256"] = _canonical_sha256(closure["dependencies"])
    source_evidence["directImportClosure"] = closure
    return {
        "schema": receipt["schema"],
        "formatVersion": receipt["formatVersion"],
        "characterClass": receipt["characterClass"],
        "skillId": receipt["skillId"],
        "inputSlot": receipt["inputSlot"],
        "policyContract": receipt["policyContract"],
        "sourceEvidence": source_evidence,
        "renderStatePolicies": receipt["renderStatePolicies"],
        "staticPermutationPolicies": receipt["staticPermutationPolicies"],
        "samplerPolicies": receipt["samplerPolicies"],
        "admission": receipt["admission"],
        "summary": receipt["summary"],
    }


def require_approved_rows(rows: list[dict[str, Any]]) -> None:
    if _canonical_sha256(policy_projection(rows)) != POLICY_PROJECTION_SHA256:
        raise ValueError("reconstructed Material policy does not match independent approval pin")


def require_approved_receipt(receipt: dict[str, Any]) -> None:
    if _canonical_sha256(approved_receipt_projection(receipt)) != APPROVED_RECEIPT_PROJECTION_SHA256:
        raise ValueError("reconstructed Material receipt boundary does not match independent approval pin")


def require_approved_oracles(receipt: dict[str, Any]) -> None:
    hlsl = receipt["hlslVerification"]
    warp = receipt["warpDescriptorVerification"]
    expected_hlsl_keys = {
        "verified", "backend", "entryPoint", "targetProfile", "compiler",
        "hlslTrackedTextSha256", "compiledDxbcSha256", "sampleCount",
        "inputBytesSha256", "outputFloat32BytesSha256", "numericTolerance",
        "maxAbsoluteError", "rowResults", "rowResultsSha256",
    }
    expected_warp_keys = {
        "verified", "backend", "featureLevel", "descriptorRowCount",
        "numericTolerance", "rowResults", "rowResultsSha256",
        "srvColorSpaceRowCount", "srvRowResults", "srvRowResultsSha256",
    }
    if set(hlsl) != expected_hlsl_keys or set(warp) != expected_warp_keys:
        raise ValueError("reconstructed Material oracle metadata schema changed")
    if (
        hlsl.get("verified") is not True
        or hlsl.get("backend") != "D3D11_WARP_COMPUTE"
        or hlsl.get("entryPoint") != "main"
        or hlsl.get("targetProfile") != "cs_5_0"
        or type(hlsl.get("numericTolerance")) is not float
        or hlsl["numericTolerance"] != 0.0
        or type(hlsl.get("maxAbsoluteError")) is not float
        or hlsl["maxAbsoluteError"] != 0.0
        or hlsl.get("compiler") != {
            "fileName": "d3dcompiler_47.dll",
            "byteSize": 4916800,
            "rawSha256": "ce013eb1639f8e2620a509e73b33029108f55a293e304e33e38b72fd65c531b8",
            "hashRole": "EXTERNAL_RAW_BYTES",
        }
    ):
        raise ValueError("reconstructed Material HLSL metadata approval changed")
    if (
        warp.get("verified") is not True
        or warp.get("backend") != "D3D11_WARP_STATE_OBJECTS"
        or type(warp.get("featureLevel")) is not int
        or warp["featureLevel"] != 45056
        or type(warp.get("numericTolerance")) is not float
        or warp["numericTolerance"] != 0.0
    ):
        raise ValueError("reconstructed Material WARP metadata approval changed")
    expected_hlsl = {
        "hlslTrackedTextSha256": HLSL_TRACKED_TEXT_SHA256,
        "compiledDxbcSha256": HLSL_DXBC_SHA256,
        "inputBytesSha256": HLSL_INPUT_BYTES_SHA256,
        "outputFloat32BytesSha256": HLSL_OUTPUT_BYTES_SHA256,
        "rowResultsSha256": HLSL_ROW_RESULTS_SHA256,
    }
    for key, expected in expected_hlsl.items():
        if hlsl.get(key) != expected:
            raise ValueError(f"reconstructed Material HLSL approval pin changed: {key}")
    if warp.get("rowResultsSha256") != WARP_DESCRIPTOR_ROW_RESULTS_SHA256:
        raise ValueError("reconstructed Material WARP descriptor approval pin changed")
    if warp.get("srvRowResultsSha256") != WARP_SRV_ROW_RESULTS_SHA256:
        raise ValueError("reconstructed Material WARP SRV approval pin changed")
