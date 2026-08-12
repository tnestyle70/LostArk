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


POLICY_PROJECTION_SHA256 = "f8043cd20bd1108cd4d632dd47d38d60035a73fd14955dbde46b1ba17b6207ac"
APPROVED_RECEIPT_PROJECTION_SHA256 = "28036d03acd89226d51a21e6d3ce255e0e9f798e9f459c993f445c4d2c3d0375"
HLSL_TRACKED_TEXT_SHA256 = "2901471f07495ff079c64ea8234ca30cd26300819d56760dbdeabae353c0b718"
HLSL_DXBC_SHA256 = "a7a4192b0d1fa70e19be8a14c8b7001d4e4020b95459bf6f88da9d3b09922bd9"
HLSL_INPUT_BYTES_SHA256 = "f24dde047c463451c6b3dd9e309818a56b396bce0b19f8c44e06965ad22679ce"
HLSL_OUTPUT_BYTES_SHA256 = "d7cb94ee9e049947e529bbb0e6ccf6120afd8a1ca1c55e0f2a631d4199ec9993"
HLSL_ROW_RESULTS_SHA256 = "251f352c3572492cf2e5c355001fbd1ce9957cd753627dbdeabc7e386252e7fa"
WARP_DESCRIPTOR_ROW_RESULTS_SHA256 = "8eec266215ab6df354babe26e192d6e0c1fd4f3dbe0631c4ef7c5ffc939f7a9c"
WARP_SRV_ROW_RESULTS_SHA256 = "ca2dcf3e7d1dbc407103eeb213b0c3e5f04fca002ae2fe8f269fc77dd5a01c67"


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
