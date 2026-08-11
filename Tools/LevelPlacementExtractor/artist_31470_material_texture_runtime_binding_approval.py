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


POLICY_RECEIPT_SHA256 = "e33f0c18d8eefa453c635ad0b8cd149f288b6505c6204d9b304ad3f1e8d623dc"
MATERIAL_CONTRACT_SHA256 = "638fae77c5805a8d33cacb69b5cdd810d40d2f304606dd56f970dd2504c1cfcb"

POLICY_TRACKED_TEXT_SHA256 = "d885e994a49ff3115a4430002f975b1d1d5824c2e269fe69b257daacba8a5fec"
MATERIAL_CONTRACT_TRACKED_TEXT_SHA256 = "a8728231ae0df26c8e203064712154989f3d42fbe5e7eaafff3db81aecda97a5"
RESOURCE_MANIFEST_TRACKED_TEXT_SHA256 = "de1d40457da5df088839aff577d1e6d838cfabae40851a346f6870ac70d8bc34"
RESOURCE_EXPORT_SOURCE_MANIFEST_CRLF_SHA256 = "f4d76e47b28c648720cfa612627508502bd54916a294e24d31c2afeb03b29c9b"
EXACT_DDS_TRACKED_TEXT_SHA256 = "3b21d1ce5d9fa6575e0d289967584e8198aec42c99aa634929bf01e2b7a97824"
CANDIDATE_TRACKED_TEXT_SHA256 = "27cfb2b81ce688b9dd55cdc53afe6e5e709cefee390dab0e9f9c5d3e9e7f244e"
RUNTIME_ORACLE_TRACKED_TEXT_SHA256 = "72c23b0cdab0026a589b3e7ab10087c96d257d2670e89d8469ad9ac70206161c"
RUNTIME_ORACLE_RECEIPT_SHA256 = "8e96b8f725c708d87221fcd9e081122956da1208db09e4982d8483499cf719e1"
ACQUISITION_TRACKED_TEXT_SHA256 = "aa94cc621f6031aa38f85a6447e7b2bf4000dbfbd90d46a61e8c1524eb2fa0bc"
ACQUISITION_RECEIPT_SHA256 = "febd8f400158b0f6916492a0bacfe60f04543601669a8381948ace2c2bd9f802"

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
    "5e46ad5c11bcfc877a5c30d554c5abcc555743a1ac8827c029212561cd8a54c6"
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
