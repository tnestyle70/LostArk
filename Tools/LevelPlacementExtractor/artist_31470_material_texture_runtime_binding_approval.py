"""Independent approval pins for Artist F Material texture runtime bindings.

The generator owns derivation.  This module owns the frozen decision boundary so
that a caller cannot change a binding, re-seal row/root digests, and have the
same validator accept the altered decision.
"""

from __future__ import annotations

import copy
import hashlib
import json
from typing import Any


POLICY_RECEIPT_SHA256 = "d753d2b51101679e1f997c8fa828ff6b0b061e782280bb9ca3fe762d8d7afec2"
MATERIAL_CONTRACT_SHA256 = "638fae77c5805a8d33cacb69b5cdd810d40d2f304606dd56f970dd2504c1cfcb"

POLICY_TRACKED_TEXT_SHA256 = "af4ec0326d50e980439eaa3601dde4a37afa282641d14d09ea626ab42f275471"
MATERIAL_CONTRACT_TRACKED_TEXT_SHA256 = "a8728231ae0df26c8e203064712154989f3d42fbe5e7eaafff3db81aecda97a5"
RESOURCE_MANIFEST_TRACKED_TEXT_SHA256 = "de1d40457da5df088839aff577d1e6d838cfabae40851a346f6870ac70d8bc34"
RESOURCE_EXPORT_SOURCE_MANIFEST_CRLF_SHA256 = "f4d76e47b28c648720cfa612627508502bd54916a294e24d31c2afeb03b29c9b"
EXACT_DDS_TRACKED_TEXT_SHA256 = "3b21d1ce5d9fa6575e0d289967584e8198aec42c99aa634929bf01e2b7a97824"
CANDIDATE_TRACKED_TEXT_SHA256 = "75f173b7b03593e50693cc4b062283a0c1993577957f4a3208476f6a27c6e10e"
RUNTIME_ORACLE_TRACKED_TEXT_SHA256 = "877bf316aded201834b28e1e9c90a249fdec118e32eddd880ef854821830e36e"
RUNTIME_ORACLE_RECEIPT_SHA256 = "1691ef6b7328abb1de83bc2841eaa501ff5ba75d67076f8812108c90adb3bfbe"
ACQUISITION_TRACKED_TEXT_SHA256 = "2268b9d559a1c0bf27fa2e1b6535b97b7973d43263547f5c119e85ea8cf3fc76"
ACQUISITION_RECEIPT_SHA256 = "573ade4469c7345f73e33bbb4262f9b8f9e770e9f51cbeb1d3564e885be2d276"

RUNTIME_COOK_RAW_SHA256 = "2e3e5db345e3e845298c6e4dd3c65d931df86d2cde10f72be4ab236265b7e04e"
RESOURCE_EXPORT_RAW_SHA256 = "9fe4ea42e7e3f60bdf8c1697211131331d8b270cddb974e11fa35d30d4279204"
SOURCE_PACK_MANIFEST_RAW_SHA256 = "8ddce11f3cdd36efc4098b127da860b3e77e0f6916263412f1089cce3967d62d"
RUNTIME_COOK_CANONICAL_SHA256 = "a0f7c98bab2ea81bb4a337167d096085e0793edfde71bb4f385db7f4e0af4891"
RESOURCE_EXPORT_CANONICAL_SHA256 = "bc918f8cec1e2e0e10eba800563a3df4b14a38844d356a1531208f44c9e51032"
SOURCE_PACK_MANIFEST_CANONICAL_SHA256 = "ae0c344f2788bbf5185d25182ee4d29d4cae17200d0e61d0cfd22b1463521a42"
RUNTIME_COOK_BYTE_COUNT = 138276
RESOURCE_EXPORT_BYTE_COUNT = 159864
SOURCE_PACK_MANIFEST_BYTE_COUNT = 270014

DEPLOYMENT_ARTIFACT_COMMIT = "01b8b8a8bace09a3576f116771daf4859aa485a3"
DEPLOYMENT_ARTIFACT_TREE = "60a229b3460e2738ef8cf4f9199a1beb8cdeb6d4"
DEPLOYMENT_RECEIPT_RELATIVE_PATH = (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.exact-dds-runtime-deployment.receipt.json"
)
DEPLOYMENT_RECEIPT_GIT_BLOB = "5d1e3766e90dc3d816b9948d85991ff57a207dca"
DEPLOYMENT_RECEIPT_TRACKED_TEXT_SHA256 = (
    "7fff1e38bce7a10b67d2b5a89c1f76f798409003b2802051f10dc177e745be18"
)
DEPLOYMENT_RECEIPT_SHA256 = (
    "de52ea2770129d254aa007a5a547ac5027977c809ace951bf6a87e8342b8c466"
)
DEPLOYMENT_APPROVAL_PROJECTION_SHA256 = (
    "419f6e2c403b0a39b49e64b2cb46b73ae48a2ed420fa0e355983da73a913d3dc"
)
DEPLOYMENT_IMPLEMENTATION_PROJECTION_SHA256 = (
    "0eda2556731377fbee03aad19ccb44c90c2092bb61eedf0ff7178540f7f9c78b"
)

# Frozen after the complete v2 receipt was rebuilt from the approved cook,
# exact-DDS deployment, and four post-verified runtime files.
APPROVED_RECEIPT_PROJECTION_SHA256 = (
    "26d5154d0cb65de0d3153de544c06bd261828fe9e0d6d27a9ff018674bbe2437"
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
    closure = candidate["sourceEvidence"]["directImportClosure"]
    closure["dependencies"] = [
        row
        for row in closure["dependencies"]
        if row["dependencyId"] != "TEXTURE_RUNTIME_BINDING_APPROVAL"
    ]
    closure["dependencyCount"] = len(closure["dependencies"])
    closure["projectionSha256"] = canonical_sha256(closure["dependencies"])
    return candidate


def receipt_projection_sha256(receipt: dict[str, Any]) -> str:
    return canonical_sha256(approved_receipt_projection(receipt))


def require_approved_receipt(receipt: dict[str, Any]) -> None:
    actual = receipt_projection_sha256(receipt)
    if actual != APPROVED_RECEIPT_PROJECTION_SHA256:
        raise ValueError(
            "Material texture runtime binding does not match independent approval pin: "
            f"expected={APPROVED_RECEIPT_PROJECTION_SHA256} actual={actual}"
        )
