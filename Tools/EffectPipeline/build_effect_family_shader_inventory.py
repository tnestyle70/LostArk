"""Classify authored source rows into material-family shader recovery status.

The recovery work needs one question answered before any shader is written: for a
given authored occurrence, which evidence already exists and which is missing.
This tool answers it by joining the authored corpus against the five contracts
that already carry the evidence, and refuses to guess when a join fails.

The primary join key is ``material.sourceMaterialPath`` compared for exact
equality with the child-parent receipt.  Only a RESOLVED receipt row may replace
the authored parent with its known denominator family.  Otherwise
``material.sourceProfile.parentMaterialPath`` is retained exactly. Leaf-name
fallback is forbidden: the corpus contains distinct families that share a leaf
name across packages, and collapsing them would merge different programs.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import sys
from typing import Any, Dict, Iterable, List, Optional, Tuple

SCHEMA = "lostark.effect-family-shader-inventory"
FORMAT_VERSION = 1

REPOSITORY_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))

SHADER_MAP_INDEX_PATH = "Data/Effects/Contracts/effect-family-shader-map-index.v1.json"
COOKED_PIXEL_SHADERS_PATH = "Data/Effects/Contracts/effect-family-cooked-pixel-shaders.v1.json"
HLSL_TRANSLATIONS_PATH = "Data/Effects/Contracts/effect-family-hlsl-translations.v1.json"
NAMED_ABI_PATH = "Data/Effects/Contracts/effect-family-named-abi.v1.json"
CHILD_PARENT_RESOLUTION_PATH = (
    "Data/Effects/Contracts/effect-child-parent-resolution.v1.json"
)

OUTPUT_PATH = "Data/Effects/Contracts/effect-family-shader-inventory.v1.json"

AUTHORED_DIRECTORY = "Data/Effects/Authored"
COOKED_SHADER_DIRECTORY = "Data/Effects/CookedShaders"
TRANSLATED_SHADER_DIRECTORY = "Data/Effects/TranslatedShaders"

SHADER_MAP_SCHEMA = "lostark.effect-family-shader-map-index"
COOKED_PIXEL_SHADERS_SCHEMA = "lostark.effect-family-cooked-pixel-shaders"
NAMED_ABI_SCHEMA = "lostark.effect-family-named-abi"
CHILD_PARENT_RESOLUTION_SCHEMA = "lostark.effect-child-parent-resolution"

COOKED_PRESENT = "COOKED_MATERIAL_MAPS_PRESENT"
COOKED_EXTRACTED = "EXTRACTED"
COOKED_BLOCKED = "BLOCKED"
TRANSLATED = "TRANSLATED"
NAMED_MAPPING_ONLY = "NAMED_LANE_IDENTITY_ONLY"
SHA256_PATTERN = re.compile(r"[0-9a-f]{64}")

# Lance Master D and F are the recovery targets. Artist F is the golden control:
# every one of its rows authors ``material.execution`` with the source profile
# disabled, so it proves the classifier keeps the two authoring paths apart.
TARGET_DOCUMENTS: Tuple[str, ...] = (
    "effect.lancemaster.skill.34110.unified.effect.json",
    "effect.lancemaster.skill.34150.unified.effect.json",
    "effect.artist.skill.31470.unified.effect.json",
)

STATUS_PROGRAM_EXACT_NAMED_ONLY = "PROGRAM_EXACT_NAMED_MAPPING_ONLY"
STATUS_PROGRAM_EXACT_NAMING_MISSING = "PROGRAM_EXACT_NAMED_MAPPING_MISSING"
STATUS_PROGRAM_PENDING_NAMED_ONLY = (
    "PROGRAM_PERMUTATION_PENDING_NAMED_MAPPING_ONLY"
)
STATUS_PROGRAM_PENDING_NAMING_MISSING = (
    "PROGRAM_PERMUTATION_PENDING_NAMED_MAPPING_MISSING"
)
STATUS_DXBC_MISSING = "SHADERMAP_FOUND_DXBC_MISSING"
STATUS_PARENT_RESOLVED_PROGRAM_MISSING = "PARENT_RESOLVED_PROGRAM_MISSING"
STATUS_PARENT_ONLY = "PARENT_ONLY"
STATUS_UNKNOWN = "UNKNOWN"
STATUS_MIXED = "MIXED_OCCURRENCE_EVIDENCE"

ALL_STATUSES: Tuple[str, ...] = (
    STATUS_PROGRAM_EXACT_NAMED_ONLY,
    STATUS_PROGRAM_EXACT_NAMING_MISSING,
    STATUS_PROGRAM_PENDING_NAMED_ONLY,
    STATUS_PROGRAM_PENDING_NAMING_MISSING,
    STATUS_DXBC_MISSING,
    STATUS_PARENT_RESOLVED_PROGRAM_MISSING,
    STATUS_PARENT_ONLY,
    STATUS_UNKNOWN,
)


class InventoryError(RuntimeError):
    """Raised when an input contract is absent or structurally unusable.

    Every raise site aborts before the output file is touched, so a failed run
    leaves the previously published contract exactly as it was.
    """


def _canonical_sha256(value: Any) -> str:
    payload = json.dumps(
        value, sort_keys=True, separators=(",", ":"), ensure_ascii=False
    )
    return hashlib.sha256(payload.encode("utf-8")).hexdigest()


def _read_json_with_identity(
    root: str,
    relative_path: str,
    *,
    require_lf: bool,
) -> Tuple[Any, Dict[str, Any]]:
    absolute_path = os.path.join(root, relative_path)
    if not os.path.isfile(absolute_path):
        raise InventoryError("required input contract is absent: " + relative_path)
    try:
        with open(absolute_path, "rb") as handle:
            payload = handle.read()
    except OSError as error:
        raise InventoryError("input could not be read: " + relative_path) from error
    if require_lf and b"\r" in payload:
        raise InventoryError("input contract is not LF-only: " + relative_path)
    try:
        document = json.loads(payload.decode("utf-8"))
    except (UnicodeError, ValueError) as error:
        raise InventoryError(
            "input contract is not valid UTF-8 JSON: " + relative_path
        ) from error
    return document, {
        "rawSha256": hashlib.sha256(payload).hexdigest(),
        "byteSize": len(payload),
    }


def _read_json(root: str, relative_path: str) -> Any:
    document, _ = _read_json_with_identity(
        root, relative_path, require_lf=False
    )
    return document


def _read_artifact(
    root: str,
    relative_path: str,
    schema: str,
) -> Tuple[Dict[str, Any], Dict[str, Any]]:
    document, identity = _read_json_with_identity(
        root, relative_path, require_lf=True
    )
    if not isinstance(document, dict):
        raise InventoryError("input contract root is not an object: " + relative_path)
    if document.get("schema") != schema:
        raise InventoryError("input contract schema is not supported: " + relative_path)
    if document.get("formatVersion") != FORMAT_VERSION:
        raise InventoryError(
            "input contract formatVersion is not supported: " + relative_path
        )
    artifact_sha = document.get("artifactSha256")
    if (
        not isinstance(artifact_sha, str)
        or SHA256_PATTERN.fullmatch(artifact_sha) is None
    ):
        raise InventoryError(
            "input contract artifactSha256 is missing or malformed: "
            + relative_path
        )
    unsigned = dict(document)
    unsigned.pop("artifactSha256", None)
    if _canonical_sha256(unsigned) != artifact_sha:
        raise InventoryError(
            "input contract artifactSha256 drifted: " + relative_path
        )
    return document, identity


def _require_dependency_pin(
    document: Dict[str, Any],
    prefix: str,
    dependency: Dict[str, Any],
    dependency_identity: Dict[str, Any],
    relative_path: str,
) -> None:
    inputs = document.get("inputs")
    if not isinstance(inputs, dict):
        raise InventoryError("input contract has no inputs object: " + relative_path)
    if inputs.get(prefix + "ArtifactSha256") != dependency.get("artifactSha256"):
        raise InventoryError(
            relative_path + " pins a different " + prefix + " artifact"
        )
    if inputs.get(prefix + "RawSha256") != dependency_identity["rawSha256"]:
        raise InventoryError(
            relative_path + " pins different " + prefix + " bytes"
        )
    pinned_size = inputs.get(prefix + "ByteSize")
    if pinned_size is not None and pinned_size != dependency_identity["byteSize"]:
        raise InventoryError(
            relative_path + " pins a different " + prefix + " byte size"
        )


def _index_by_parent(document: Any, relative_path: str) -> Dict[str, Any]:
    families = document.get("families") if isinstance(document, dict) else None
    if not isinstance(families, list):
        raise InventoryError("input contract has no families array: " + relative_path)
    indexed: Dict[str, Any] = {}
    for family in families:
        if not isinstance(family, dict):
            raise InventoryError("family row is not an object: " + relative_path)
        parent = family.get("parentMaterialPath")
        if not isinstance(parent, str) or not parent:
            raise InventoryError(
                "family row has no parentMaterialPath: " + relative_path
            )
        # A duplicate key would silently drop evidence, so it is an error rather
        # than a last-writer-wins overwrite.
        if parent in indexed:
            raise InventoryError(
                "duplicate parentMaterialPath in " + relative_path + ": " + parent
            )
        indexed[parent] = family
    return indexed


def _require_sha256(value: Any, description: str) -> str:
    if not isinstance(value, str) or SHA256_PATTERN.fullmatch(value) is None:
        raise InventoryError(description + " is missing or malformed")
    return value


def _index_translations(
    document: Any,
    relative_path: str,
    expected_digests: Iterable[str],
) -> Dict[str, Any]:
    if not isinstance(document, list):
        raise InventoryError("translation contract is not an array: " + relative_path)
    indexed: Dict[str, Any] = {}
    function_names = set()
    for entry in document:
        if not isinstance(entry, dict):
            raise InventoryError("translation row is not an object: " + relative_path)
        if entry.get("status") != TRANSLATED:
            raise InventoryError("translation row is not TRANSLATED: " + relative_path)
        digest = _require_sha256(
            entry.get("dxbcSha256"), "translation row dxbcSha256"
        )
        if entry.get("dxbc") != digest + ".dxbc":
            raise InventoryError(
                "translation DXBC filename differs from its digest: " + digest
            )
        if digest in indexed:
            raise InventoryError(
                "duplicate dxbcSha256 in " + relative_path + ": " + digest
            )
        function_name = entry.get("functionName")
        if not isinstance(function_name, str) or not function_name:
            raise InventoryError("translation row has no functionName: " + relative_path)
        folded_name = function_name.casefold()
        if folded_name in function_names:
            raise InventoryError(
                "duplicate translation functionName in "
                + relative_path
                + ": "
                + function_name
            )
        function_names.add(folded_name)
        _require_sha256(entry.get("hlslSha256"), "translation row hlslSha256")
        indexed[digest] = entry
    expected = set(expected_digests)
    actual = set(indexed)
    if actual != expected:
        raise InventoryError(
            "translation denominator differs from EXTRACTED cooked programs: "
            + "missing="
            + str(len(expected - actual))
            + " extra="
            + str(len(actual - expected))
        )
    return indexed


def _validate_program_artifacts(
    root: str,
    translations: Dict[str, Any],
    digest_sizes: Dict[str, int],
) -> Tuple[Dict[str, Any], Dict[str, Any]]:
    cooked_identities: Dict[str, Any] = {}
    hlsl_identities: Dict[str, Any] = {}
    expected_hlsli = set()

    for digest, entry in translations.items():
        dxbc_relative = COOKED_SHADER_DIRECTORY + "/" + digest + ".dxbc"
        dxbc_path = os.path.join(root, dxbc_relative)
        if not os.path.isfile(dxbc_path):
            raise InventoryError("cooked DXBC artifact is absent: " + dxbc_relative)
        with open(dxbc_path, "rb") as handle:
            dxbc_payload = handle.read()
        if len(dxbc_payload) != digest_sizes[digest]:
            raise InventoryError("cooked DXBC byte size drifted: " + dxbc_relative)
        if hashlib.sha256(dxbc_payload).hexdigest() != digest:
            raise InventoryError("cooked DXBC raw SHA-256 drifted: " + dxbc_relative)
        if not dxbc_payload.startswith(b"DXBC"):
            raise InventoryError("cooked shader has no DXBC signature: " + dxbc_relative)
        cooked_identities[digest] = {
            "rawSha256": digest,
            "byteSize": len(dxbc_payload),
        }

        function_name = entry["functionName"]
        hlsli_name = function_name + ".hlsli"
        expected_hlsli.add(hlsli_name)
        hlsli_relative = TRANSLATED_SHADER_DIRECTORY + "/" + hlsli_name
        hlsli_path = os.path.join(root, hlsli_relative)
        if not os.path.isfile(hlsli_path):
            raise InventoryError("translated HLSLI artifact is absent: " + hlsli_relative)
        with open(hlsli_path, "rb") as handle:
            hlsli_payload = handle.read()
        if b"\r" in hlsli_payload:
            raise InventoryError("translated HLSLI is not LF-only: " + hlsli_relative)
        hlsli_sha = hashlib.sha256(hlsli_payload).hexdigest()
        if hlsli_sha != entry["hlslSha256"]:
            raise InventoryError("translated HLSLI raw SHA-256 drifted: " + hlsli_relative)
        try:
            hlsli_source = hlsli_payload.decode("utf-8")
        except UnicodeError as error:
            raise InventoryError(
                "translated HLSLI is not UTF-8: " + hlsli_relative
            ) from error
        if re.search(r"\b" + re.escape(function_name) + r"\s*\(", hlsli_source) is None:
            raise InventoryError(
                "translated HLSLI does not declare its function: " + hlsli_relative
            )
        hlsl_identities[hlsli_name] = {
            "rawSha256": hlsli_sha,
            "byteSize": len(hlsli_payload),
        }

    translated_directory = os.path.join(root, TRANSLATED_SHADER_DIRECTORY)
    if not os.path.isdir(translated_directory):
        raise InventoryError(
            "translated shader directory is absent: " + TRANSLATED_SHADER_DIRECTORY
        )
    actual_hlsli = {
        name
        for name in os.listdir(translated_directory)
        if name.lower().endswith(".hlsli")
        and os.path.isfile(os.path.join(translated_directory, name))
    }
    if actual_hlsli != expected_hlsli:
        raise InventoryError(
            "translated HLSLI file set differs from the report: missing="
            + str(len(expected_hlsli - actual_hlsli))
            + " extra="
            + str(len(actual_hlsli - expected_hlsli))
        )
    return cooked_identities, hlsl_identities


def _validate_shader_map_summary(
    document: Dict[str, Any], indexed: Dict[str, Any]
) -> None:
    summary = document.get("summary")
    if not isinstance(summary, dict):
        raise InventoryError("shader-map index has no summary object")
    if summary.get("parentMaterialCount") != len(indexed):
        raise InventoryError("shader-map index parentMaterialCount is inconsistent")


def _validate_cooked_contract(
    document: Dict[str, Any],
    indexed: Dict[str, Any],
    shader_map: Dict[str, Any],
) -> Dict[str, int]:
    expected_parents = {
        parent
        for parent, row in shader_map.items()
        if row.get("cookedEvidence") == COOKED_PRESENT
    }
    if set(indexed) != expected_parents:
        raise InventoryError(
            "cooked family denominator differs from shader-map cooked evidence"
        )
    digest_sizes: Dict[str, int] = {}
    extracted_count = 0
    for parent, row in indexed.items():
        status = row.get("status")
        if status not in (COOKED_EXTRACTED, COOKED_BLOCKED):
            raise InventoryError("cooked family has unknown status: " + parent)
        if status == COOKED_BLOCKED:
            blocker = row.get("blocker")
            if not isinstance(blocker, str) or not blocker.strip():
                raise InventoryError("BLOCKED cooked family has no blocker: " + parent)
            continue
        extracted_count += 1
        digest = _require_sha256(
            row.get("dxbcSha256"), "EXTRACTED cooked family dxbcSha256"
        )
        size = row.get("dxbcByteSize")
        if not isinstance(size, int) or isinstance(size, bool) or size <= 0:
            raise InventoryError(
                "EXTRACTED cooked family dxbcByteSize is invalid: " + parent
            )
        previous = digest_sizes.setdefault(digest, size)
        if previous != size:
            raise InventoryError(
                "shared cooked DXBC has conflicting byte sizes: " + digest
            )
        carrier = row.get("carrier")
        if not isinstance(carrier, str) or not carrier:
            raise InventoryError("EXTRACTED cooked family has no carrier: " + parent)
        selection = row.get("permutationSelection")
        permutation_count = row.get("permutationCount")
        if (
            not isinstance(permutation_count, int)
            or isinstance(permutation_count, bool)
            or permutation_count <= 0
        ):
            raise InventoryError(
                "EXTRACTED cooked family permutationCount is invalid: " + parent
            )
        if selection == "SINGLE_PERMUTATION_FAMILY":
            if permutation_count != 1:
                raise InventoryError(
                    "single-permutation cooked family has count != 1: " + parent
                )
        elif selection == "CHILD_MIC_ENGINE_EQUALITY":
            child = row.get("childMaterialPath")
            if not isinstance(child, str) or not child:
                raise InventoryError(
                    "child-selected cooked family has no childMaterialPath: " + parent
                )
        else:
            raise InventoryError(
                "EXTRACTED cooked family has unknown permutationSelection: " + parent
            )
    summary = document.get("summary")
    if not isinstance(summary, dict):
        raise InventoryError("cooked contract has no summary object")
    expected_counts = {
        "familyCount": len(indexed),
        "extractedCount": extracted_count,
        "blockedCount": len(indexed) - extracted_count,
    }
    for key, value in expected_counts.items():
        if summary.get(key) != value:
            raise InventoryError("cooked summary." + key + " is inconsistent")
    return digest_sizes


def _validate_named_abi_contract(
    document: Dict[str, Any],
    indexed: Dict[str, Any],
    cooked: Dict[str, Any],
    digest_sizes: Dict[str, int],
) -> None:
    extracted = {
        parent: row
        for parent, row in cooked.items()
        if row.get("status") == COOKED_EXTRACTED
    }
    if set(indexed) != set(extracted):
        raise InventoryError(
            "named ABI family denominator differs from EXTRACTED cooked families"
        )
    resolved = 0
    blocked = 0
    blocked_parents = []
    for parent, row in indexed.items():
        admits = row.get("admits")
        if admits == NAMED_MAPPING_ONLY:
            resolved += 1
            if row.get("dxbcSha256") != extracted[parent].get("dxbcSha256"):
                raise InventoryError("named ABI DXBC identity drifted: " + parent)
            continue
        if row.get("status") == COOKED_BLOCKED:
            blocked += 1
            blocked_parents.append(parent)
            blocker = row.get("blocker")
            if not isinstance(blocker, dict):
                raise InventoryError(
                    "BLOCKED named ABI family has no structured blocker: " + parent
                )
            reason_code = blocker.get("reasonCode")
            candidate_count = blocker.get("candidateCount")
            if not isinstance(reason_code, str) or not reason_code.strip():
                raise InventoryError(
                    "BLOCKED named ABI family has no reasonCode: " + parent
                )
            if (
                not isinstance(candidate_count, int)
                or isinstance(candidate_count, bool)
                or candidate_count <= 0
            ):
                raise InventoryError(
                    "BLOCKED named ABI family candidateCount is invalid: " + parent
                )
            continue
        raise InventoryError("named ABI family has unknown evidence state: " + parent)

    summary = document.get("summary")
    if not isinstance(summary, dict):
        raise InventoryError("named ABI contract has no summary object")
    expected_counts = {
        "familyCount": len(indexed),
        "resolvedNamedMappingCount": resolved,
        "blockedCount": blocked,
    }
    for key, value in expected_counts.items():
        if summary.get(key) != value:
            raise InventoryError("named ABI summary." + key + " is inconsistent")
    expected_blocked_parents = sorted(blocked_parents)
    if summary.get("blockedParents") != expected_blocked_parents:
        raise InventoryError("named ABI summary.blockedParents is inconsistent")
    if summary.get("blockedParentSetSha256") != _canonical_sha256(
        expected_blocked_parents
    ):
        raise InventoryError("named ABI blockedParentSetSha256 is inconsistent")

    inputs = document.get("inputs")
    if not isinstance(inputs, dict):
        raise InventoryError("named ABI contract has no inputs object")
    if inputs.get("cookedShaderProgramCount") != len(digest_sizes):
        raise InventoryError("named ABI cookedShaderProgramCount is inconsistent")
    identities = {
        digest: {"rawSha256": digest, "byteSize": size}
        for digest, size in digest_sizes.items()
    }
    if inputs.get("cookedShaderSetSha256") != _canonical_sha256(identities):
        raise InventoryError("named ABI cookedShaderSetSha256 is inconsistent")


def _validate_child_parent_contract(
    document: Dict[str, Any],
    shader_map: Dict[str, Any],
    cooked: Dict[str, Any],
    abi: Dict[str, Any],
) -> Dict[str, Dict[str, Any]]:
    identity = document.get("identity")
    if not isinstance(identity, dict):
        raise InventoryError("child-parent contract has no identity object")
    if identity.get("scope") != "AUTHORED_CORPUS_ORPHAN_CHILD_MATERIALS":
        raise InventoryError("child-parent contract scope is not supported")
    if identity.get("admits") != "PARENT_MATERIAL_JOIN_KEY_ONLY":
        raise InventoryError("child-parent contract overstates its admission")

    children = document.get("children")
    if not isinstance(children, list):
        raise InventoryError("child-parent contract has no children array")
    indexed: Dict[str, Dict[str, Any]] = {}
    resolved_count = 0
    blocked_count = 0
    orphan_element_count = 0
    resolved_by_counts: Dict[str, int] = {}
    blocker_counts: Dict[str, int] = {}
    recovered_parent_counts: Dict[str, int] = {}
    recovered_parent_evidence: Dict[str, Tuple[Any, ...]] = {}
    for row in children:
        if not isinstance(row, dict):
            raise InventoryError("child-parent row is not an object")
        child = row.get("childMaterialPath")
        if not isinstance(child, str) or not child:
            raise InventoryError("child-parent row has no childMaterialPath")
        if child in indexed:
            raise InventoryError("duplicate childMaterialPath in child-parent contract: " + child)
        row_sha = row.get("rowSha256")
        unsigned = dict(row)
        unsigned.pop("rowSha256", None)
        if not isinstance(row_sha, str) or row_sha != _canonical_sha256(unsigned):
            raise InventoryError("child-parent rowSha256 drifted: " + child)

        element_count = row.get("elementCount")
        asset_count = row.get("effectAssetIdCount")
        if (
            not isinstance(element_count, int)
            or isinstance(element_count, bool)
            or element_count <= 0
            or not isinstance(asset_count, int)
            or isinstance(asset_count, bool)
            or asset_count <= 0
            or asset_count > element_count
        ):
            raise InventoryError(
                "child-parent occurrence counts are inconsistent: " + child
            )
        orphan_element_count += element_count

        status = row.get("status")
        if status == "BLOCKED":
            blocked_count += 1
            blocker = row.get("blocker")
            if not isinstance(blocker, str) or not blocker:
                raise InventoryError("BLOCKED child-parent row has no blocker: " + child)
            blocker_counts[blocker] = blocker_counts.get(blocker, 0) + 1
        elif status == "RESOLVED":
            resolved_count += 1
            resolved_by = row.get("resolvedBy")
            if resolved_by not in ("DECLARED_PACKAGE_EXPORT", "LEAF_NAME_SEARCH"):
                raise InventoryError(
                    "RESOLVED child-parent row has unsupported resolution evidence: "
                    + child
                )
            resolved_by_counts[resolved_by] = resolved_by_counts.get(resolved_by, 0) + 1
            parent_chain = row.get("parentChain")
            if not isinstance(parent_chain, list) or not all(
                isinstance(path, str) and bool(path) for path in parent_chain
            ):
                raise InventoryError(
                    "RESOLVED child-parent row has malformed parentChain: " + child
                )
            canonical = row.get("canonicalParentMaterialPath")
            if not isinstance(canonical, str) or not canonical:
                raise InventoryError(
                    "RESOLVED child-parent row has no canonical parent: " + child
                )
            already_known = row.get("familyAlreadyInDenominator")
            if not isinstance(already_known, bool):
                raise InventoryError(
                    "RESOLVED child-parent row has invalid denominator flag: " + child
                )
            known = row.get("knownFamilyPath")
            if already_known:
                if not isinstance(known, str) or not known:
                    raise InventoryError(
                        "known child-parent row has no knownFamilyPath: " + child
                    )
                if known not in shader_map:
                    raise InventoryError(
                        "child-parent known family is absent from shader-map denominator: "
                        + child
                    )
                cooked_row = cooked.get(known)
                expected_cooked_status = (
                    cooked_row.get("status")
                    if isinstance(cooked_row, dict)
                    else "ABSENT"
                )
                if row.get("cookedPixelShaderStatus") != expected_cooked_status:
                    raise InventoryError(
                        "child-parent cooked status drifted: " + child
                    )
                named_closed = (
                    isinstance(abi.get(known), dict)
                    and abi[known].get("admits") == NAMED_MAPPING_ONLY
                )
                if row.get("namedAbiClosed") is not named_closed:
                    raise InventoryError("child-parent named ABI flag drifted: " + child)
            elif known is not None:
                raise InventoryError(
                    "new child-parent family unexpectedly has knownFamilyPath: " + child
                )
            else:
                if canonical in shader_map:
                    raise InventoryError(
                        "new child-parent family already exists in shader-map denominator: "
                        + child
                    )
                if row.get("cookedPixelShaderStatus") != "ABSENT":
                    raise InventoryError(
                        "new child-parent family has non-ABSENT cooked status: " + child
                    )
                if row.get("namedAbiClosed") is not False:
                    raise InventoryError(
                        "new child-parent family claims named ABI closure: " + child
                    )
            evidence = (
                known,
                row.get("cookedPixelShaderStatus"),
                row.get("namedAbiClosed"),
                already_known,
            )
            previous = recovered_parent_evidence.setdefault(canonical, evidence)
            if previous != evidence:
                raise InventoryError(
                    "children disagree on recovered family evidence: " + canonical
                )
            recovered_parent_counts[canonical] = (
                recovered_parent_counts.get(canonical, 0) + element_count
            )
        else:
            raise InventoryError("child-parent row has unknown status: " + child)
        indexed[child] = row

    families = document.get("families")
    if not isinstance(families, list):
        raise InventoryError("child-parent contract has no families array")
    family_by_parent: Dict[str, Dict[str, Any]] = {}
    for row in families:
        if not isinstance(row, dict):
            raise InventoryError("child-parent family row is not an object")
        parent = row.get("canonicalParentMaterialPath")
        if not isinstance(parent, str) or not parent:
            raise InventoryError("child-parent family row has no canonical parent")
        if parent in family_by_parent:
            raise InventoryError("duplicate family in child-parent contract: " + parent)
        row_sha = row.get("rowSha256")
        unsigned = dict(row)
        unsigned.pop("rowSha256", None)
        if not isinstance(row_sha, str) or row_sha != _canonical_sha256(unsigned):
            raise InventoryError("child-parent family rowSha256 drifted: " + parent)
        family_by_parent[parent] = row

    if set(family_by_parent) != set(recovered_parent_counts):
        raise InventoryError(
            "child-parent family set differs from resolved child parents"
        )
    for parent, row in family_by_parent.items():
        if row.get("recoveredElementCount") != recovered_parent_counts[parent]:
            raise InventoryError(
                "child-parent family recoveredElementCount is inconsistent: " + parent
            )
        actual_evidence = (
            row.get("knownFamilyPath"),
            row.get("cookedPixelShaderStatus"),
            row.get("namedAbiClosed"),
            row.get("alreadyInDenominator"),
        )
        if actual_evidence != recovered_parent_evidence[parent]:
            raise InventoryError(
                "child-parent family evidence differs from its children: " + parent
            )

    summary = document.get("summary")
    if not isinstance(summary, dict):
        raise InventoryError("child-parent contract has no summary object")
    expected_counts = {
        "parentLostWithChildPathElementCount": orphan_element_count,
        "distinctOrphanChildCount": len(indexed),
        "resolvedChildCount": resolved_count,
        "resolvedByCounts": dict(sorted(resolved_by_counts.items())),
        "blockedChildCount": blocked_count,
        "recoveredElementCount": sum(recovered_parent_counts.values()),
        "recoveredParentMaterialCount": len(family_by_parent),
        "recoveredParentsAlreadyInDenominator": sum(
            row.get("alreadyInDenominator") is True
            for row in family_by_parent.values()
        ),
        "recoveredParentsNewToDenominator": sum(
            row.get("alreadyInDenominator") is False
            for row in family_by_parent.values()
        ),
        "recoveredElementsWithExtractedDxbc": sum(
            row["recoveredElementCount"]
            for row in family_by_parent.values()
            if row.get("cookedPixelShaderStatus") == COOKED_EXTRACTED
        ),
        "blockerCounts": dict(sorted(blocker_counts.items())),
    }
    for key, value in expected_counts.items():
        if summary.get(key) != value:
            raise InventoryError("child-parent summary." + key + " is inconsistent")
    parent_retained = summary.get("parentRetainedElementCount")
    parent_lost_without_child = summary.get(
        "parentLostWithoutChildPathElementCount"
    )
    if any(
        not isinstance(value, int) or isinstance(value, bool) or value < 0
        for value in (parent_retained, parent_lost_without_child)
    ):
        raise InventoryError("child-parent retained/lost summary counts are malformed")
    parent_lost = orphan_element_count + parent_lost_without_child
    if summary.get("parentLostElementCount") != parent_lost:
        raise InventoryError("child-parent summary.parentLostElementCount is inconsistent")
    if summary.get("authoredElementCount") != parent_retained + parent_lost:
        raise InventoryError("child-parent summary.authoredElementCount is inconsistent")
    return indexed


def _resolve_parent_material_path(
    authored_parent: str,
    source_material_path: str,
    child_parent: Dict[str, Dict[str, Any]],
) -> Tuple[str, str, Optional[str]]:
    row = child_parent.get(source_material_path)
    if not isinstance(row, dict) or row.get("status") != "RESOLVED":
        return authored_parent, "AUTHORED_PARENT_EXACT", None
    if row.get("familyAlreadyInDenominator") is True:
        return (
            row["knownFamilyPath"],
            "CHILD_PARENT_KNOWN_FAMILY_EXACT",
            row.get("rowSha256"),
        )
    return (
        row["canonicalParentMaterialPath"],
        "CHILD_PARENT_NEW_FAMILY_EXACT",
        row.get("rowSha256"),
    )


def _collect_source_rows(
    root: str, document_names: Iterable[str]
) -> Tuple[
    List[Dict[str, Any]],
    Dict[str, Dict[str, int]],
    List[str],
    List[Dict[str, Any]],
]:
    """Return occurrence inputs, per-document path counts, and load order.

    ``sourceProfile.enabled`` is the gate. Rows that author ``material.execution``
    instead run the typed executor path and never consult a family, so counting
    them here would overstate what family recovery can reach.
    """

    occurrences: List[Dict[str, Any]] = []
    per_document: Dict[str, Dict[str, int]] = {}
    loaded: List[str] = []
    identities: List[Dict[str, Any]] = []
    for name in document_names:
        relative_path = AUTHORED_DIRECTORY + "/" + name
        document, _ = _read_json_with_identity(
            root, relative_path, require_lf=False
        )
        elements = document.get("elements") if isinstance(document, dict) else None
        if not isinstance(elements, list):
            raise InventoryError("authored document has no elements: " + relative_path)
        loaded.append(name)
        identities.append(
            {
                "document": name,
                # Authored JSON is not LF-pinned repository-wide.  Canonical
                # JSON identity detects semantic drift without making this
                # contract depend on checkout line endings.
                "canonicalSha256": _canonical_sha256(document),
                "elementCount": len(elements),
            }
        )
        source_row_count = 0
        execution_row_count = 0
        for element in elements:
            if not isinstance(element, dict):
                raise InventoryError("element is not an object: " + relative_path)
            material = element.get("material")
            if not isinstance(material, dict):
                continue
            profile = material.get("sourceProfile")
            source_enabled = isinstance(profile, dict) and profile.get("enabled") is True
            execution = material.get("execution")
            execution_enabled = (
                isinstance(execution, dict) and execution.get("enabled") is True
            )
            if source_enabled and execution_enabled:
                raise InventoryError(
                    "material enables sourceProfile and execution together: "
                    + relative_path
                )
            if execution_enabled:
                execution_row_count += 1
            if not source_enabled:
                continue
            source_row_count += 1
            parent = profile.get("parentMaterialPath")
            source_material = material.get("sourceMaterialPath")
            source_recipe = element.get("sourceRecipe")
            carrier = (
                source_recipe.get("rendererShape")
                if isinstance(source_recipe, dict)
                else None
            )
            element_id = element.get("id")
            if not isinstance(element_id, str) or not element_id:
                raise InventoryError("source row has no stable id: " + relative_path)
            occurrences.append(
                {
                    "document": name,
                    "elementId": element_id,
                    "authoredParentMaterialPath": (
                        parent if isinstance(parent, str) else ""
                    ),
                    "sourceMaterialPath": (
                        source_material if isinstance(source_material, str) else ""
                    ),
                    "carrier": carrier if isinstance(carrier, str) else "",
                }
            )
        per_document[name] = {
            "sourceRowCount": source_row_count,
            "executionRowCount": execution_row_count,
        }
    return occurrences, per_document, loaded, identities


def _named_blocker_message(abi_row: Optional[Dict[str, Any]]) -> str:
    if not isinstance(abi_row, dict):
        return "named ABI contract has no row for this family"
    blocker = abi_row.get("blocker")
    if isinstance(blocker, dict):
        reason = str(blocker.get("reasonCode") or "NAMED_MAPPING_BLOCKED")
        candidate_count = blocker.get("candidateCount")
        return reason + " candidateCount=" + str(candidate_count)
    if isinstance(blocker, str) and blocker:
        return blocker
    return "named lane identity is blocked"


def _classify(
    parent_material_path: str,
    source_material_path: str,
    source_carrier: str,
    parent_resolution: str,
    shader_map_row: Optional[Dict[str, Any]],
    cooked_row: Optional[Dict[str, Any]],
    abi_row: Optional[Dict[str, Any]],
    translations: Dict[str, Any],
) -> Tuple[str, str, Optional[Dict[str, Any]]]:
    """Return occurrence evidence without promoting a family representative.

    Program identity, named-lane identity, runtime ABI closure and Product
    admission are deliberately separate gates.  The named-ABI receipt proves
    only the second gate, so no status returned here claims a renderable
    occurrence or a Product result.
    """

    if not parent_material_path:
        return (
            STATUS_UNKNOWN,
            "authored source profile carries no parentMaterialPath",
            None,
        )
    if shader_map_row is None:
        if parent_resolution == "CHILD_PARENT_NEW_FAMILY_EXACT":
            return (
                STATUS_PARENT_RESOLVED_PROGRAM_MISSING,
                "child-parent receipt resolved a new family outside the current "
                "shader-map/cooked denominator",
                None,
            )
        return (
            STATUS_UNKNOWN,
            "parent material path is absent from the shader map index",
            None,
        )

    cooked_status = cooked_row.get("status") if cooked_row else None

    if cooked_status == COOKED_BLOCKED:
        return (
            STATUS_DXBC_MISSING,
            str(cooked_row.get("blocker") or "cooked pixel shader extraction is blocked"),
            None,
        )

    if cooked_status != COOKED_EXTRACTED:
        resolution = shader_map_row.get("resolution")
        resolved_by = (
            resolution.get("resolvedBy") if isinstance(resolution, dict) else None
        )
        return (
            STATUS_PARENT_ONLY,
            "shader map resolution="
            + str(resolved_by)
            + ", cookedEvidence="
            + str(shader_map_row.get("cookedEvidence")),
            None,
        )

    digest = cooked_row.get("dxbcSha256") if cooked_row else None
    translation = translations.get(digest)
    if translation is None:
        raise InventoryError(
            "validated cooked program has no literal translation: "
            + parent_material_path
        )

    selection = cooked_row.get("permutationSelection")
    selected_child = cooked_row.get("childMaterialPath")
    carrier_matches = (
        isinstance(cooked_row.get("carrier"), str)
        and bool(source_carrier)
        and cooked_row.get("carrier") == source_carrier
    )
    if selection == "SINGLE_PERMUTATION_FAMILY":
        permutation_matches = True
        mismatch = ""
    elif selection == "CHILD_MIC_ENGINE_EQUALITY":
        permutation_matches = (
            isinstance(selected_child, str)
            and bool(selected_child)
            and source_material_path == selected_child
        )
        mismatch = (
            "source child does not match the cooked representative permutation"
        )
    else:
        raise InventoryError(
            "EXTRACTED cooked family has unknown permutationSelection: "
            + parent_material_path
        )

    occurrence_exact = permutation_matches and carrier_matches
    named_closed = abi_row is not None and abi_row.get("admits") == NAMED_MAPPING_ONLY
    if occurrence_exact and not named_closed:
        return (
            STATUS_PROGRAM_EXACT_NAMING_MISSING,
            _named_blocker_message(abi_row),
            translation,
        )
    if occurrence_exact:
        return (
            STATUS_PROGRAM_EXACT_NAMED_ONLY,
            "occurrence child/static permutation and carrier match the literal "
            "program; runtime ABI packet, adapter, and Product remain unproven",
            translation,
        )

    blocker_parts = []
    if not permutation_matches:
        blocker_parts.append(mismatch)
    if not carrier_matches:
        blocker_parts.append(
            "source carrier="
            + str(source_carrier or None)
            + " differs from cooked carrier="
            + str(cooked_row.get("carrier"))
        )
    if not named_closed:
        blocker_parts.append(_named_blocker_message(abi_row))
        status = STATUS_PROGRAM_PENDING_NAMING_MISSING
    else:
        status = STATUS_PROGRAM_PENDING_NAMED_ONLY
    return status, "; ".join(blocker_parts), translation


def _build_row(
    parent_material_path: str,
    occurrences: List[Dict[str, Any]],
    shader_map_row: Optional[Dict[str, Any]],
    cooked_row: Optional[Dict[str, Any]],
    abi_row: Optional[Dict[str, Any]],
    translations: Dict[str, Any],
) -> Dict[str, Any]:
    summary = abi_row.get("summary") if abi_row else None
    summary = summary if isinstance(summary, dict) else {}
    texture_slots = abi_row.get("textureSlots") if abi_row else None
    texture_slots = texture_slots if isinstance(texture_slots, list) else []
    documents: Dict[str, int] = {}
    source_parents: Dict[str, int] = {}
    source_materials: Dict[str, int] = {}
    parent_resolutions: Dict[str, int] = {}
    evidence: Dict[str, Dict[str, Any]] = {}
    for occurrence in occurrences:
        document = occurrence["document"]
        documents[document] = documents.get(document, 0) + 1
        authored_parent = occurrence["authoredParentMaterialPath"]
        source_parents[authored_parent] = source_parents.get(authored_parent, 0) + 1
        source_material = occurrence["sourceMaterialPath"]
        source_materials[source_material] = source_materials.get(source_material, 0) + 1
        resolution = occurrence["parentResolution"]
        parent_resolutions[resolution] = parent_resolutions.get(resolution, 0) + 1
        status = occurrence["status"]
        bucket = evidence.setdefault(
            status, {"occurrenceCount": 0, "blockers": set()}
        )
        bucket["occurrenceCount"] += 1
        if occurrence["blocker"]:
            bucket["blockers"].add(occurrence["blocker"])

    statuses = sorted(evidence)
    exact_count = sum(
        evidence[status]["occurrenceCount"]
        for status in (
            STATUS_PROGRAM_EXACT_NAMED_ONLY,
            STATUS_PROGRAM_EXACT_NAMING_MISSING,
        )
        if status in evidence
    )
    pending_count = sum(
        evidence[status]["occurrenceCount"]
        for status in (
            STATUS_PROGRAM_PENDING_NAMED_ONLY,
            STATUS_PROGRAM_PENDING_NAMING_MISSING,
        )
        if status in evidence
    )
    if exact_count and pending_count:
        program_evidence = "MIXED_EXACT_AND_FAMILY_REPRESENTATIVE"
    elif exact_count:
        program_evidence = "OCCURRENCE_EXACT_LITERAL_TRANSLATION"
    elif pending_count:
        program_evidence = "FAMILY_REPRESENTATIVE_LITERAL_TRANSLATION_ONLY"
    else:
        program_evidence = "NOT_PROVEN"

    digest = (cooked_row or {}).get("dxbcSha256")
    translation = translations.get(digest)
    row: Dict[str, Any] = {
        "parentMaterialPath": parent_material_path,
        "status": statuses[0] if len(statuses) == 1 else STATUS_MIXED,
        "blocker": (
            "; ".join(sorted(evidence[statuses[0]]["blockers"]))
            if len(statuses) == 1
            else "family contains multiple occurrence evidence classes"
        ),
        "occurrenceCount": len(occurrences),
        "documents": [
            {"document": name, "occurrenceCount": documents[name]}
            for name in sorted(documents)
        ],
        "sourceParentMaterialPaths": [
            {"path": path, "occurrenceCount": source_parents[path]}
            for path in sorted(source_parents)
        ],
        "sourceMaterialPaths": [
            {"path": path, "occurrenceCount": source_materials[path]}
            for path in sorted(source_materials)
        ],
        "parentResolutions": [
            {"resolvedBy": name, "occurrenceCount": parent_resolutions[name]}
            for name in sorted(parent_resolutions)
        ],
        "evidenceClasses": [
            {
                "status": status,
                "occurrenceCount": evidence[status]["occurrenceCount"],
                "blockers": sorted(evidence[status]["blockers"]),
            }
            for status in statuses
        ],
        "programEvidence": program_evidence,
        "programExactOccurrenceCount": exact_count,
        "programPermutationPendingOccurrenceCount": pending_count,
        "namedMappingEvidence": (
            "NAMED_LANE_IDENTITY_ONLY"
            if isinstance(abi_row, dict)
            and abi_row.get("admits") == NAMED_MAPPING_ONLY
            else "MISSING_OR_BLOCKED"
        ),
        "runtimeAbiClosure": "NOT_PROVEN",
        "productAdmission": "NOT_ADMITTED",
        "carrier": (cooked_row or {}).get("carrier"),
        "childMaterialPath": (cooked_row or {}).get("childMaterialPath"),
        "permutationSelection": (cooked_row or {}).get("permutationSelection"),
        "permutationCount": (cooked_row or {}).get("permutationCount"),
        "dxbcSha256": digest,
        "translatedFunctionName": (translation or {}).get("functionName"),
        "instructionCount": (translation or {}).get("instructionCount"),
        "scalarLaneCount": summary.get("scalarLaneCount"),
        "vectorLaneCount": summary.get("vectorLaneCount"),
        "textureSlots": [
            {
                "textureRegister": slot.get("textureRegister"),
                "samplerRegister": slot.get("samplerRegister"),
                "parameterName": slot.get("parameterName"),
                "referencedTextureIndex": slot.get("referencedTextureIndex"),
            }
            for slot in texture_slots
        ],
        "timeDependentRegisters": list(summary.get("timeDependentRegisters") or []),
    }
    if shader_map_row is not None:
        resolution = shader_map_row.get("resolution")
        row["shaderMapResolvedBy"] = (
            resolution.get("resolvedBy") if isinstance(resolution, dict) else None
        )
        row["shaderMapCookedEvidence"] = shader_map_row.get("cookedEvidence")
        row["shaderMapOccurrenceCount"] = shader_map_row.get("occurrenceCount")
    else:
        row["shaderMapResolvedBy"] = None
        row["shaderMapCookedEvidence"] = None
        row["shaderMapOccurrenceCount"] = None
    return row


def build_inventory(
    root: str, document_names: Iterable[str] = TARGET_DOCUMENTS
) -> Dict[str, Any]:
    shader_map_document, shader_map_identity = _read_artifact(
        root, SHADER_MAP_INDEX_PATH, SHADER_MAP_SCHEMA
    )
    cooked_document, cooked_identity = _read_artifact(
        root, COOKED_PIXEL_SHADERS_PATH, COOKED_PIXEL_SHADERS_SCHEMA
    )
    abi_document, abi_identity = _read_artifact(
        root, NAMED_ABI_PATH, NAMED_ABI_SCHEMA
    )
    child_parent_document, child_parent_identity = _read_artifact(
        root, CHILD_PARENT_RESOLUTION_PATH, CHILD_PARENT_RESOLUTION_SCHEMA
    )
    translations_document, translations_identity = _read_json_with_identity(
        root, HLSL_TRANSLATIONS_PATH, require_lf=True
    )

    shader_map = _index_by_parent(shader_map_document, SHADER_MAP_INDEX_PATH)
    cooked = _index_by_parent(cooked_document, COOKED_PIXEL_SHADERS_PATH)
    abi = _index_by_parent(abi_document, NAMED_ABI_PATH)
    _validate_shader_map_summary(shader_map_document, shader_map)
    _require_dependency_pin(
        cooked_document,
        "shaderMap",
        shader_map_document,
        shader_map_identity,
        COOKED_PIXEL_SHADERS_PATH,
    )
    digest_sizes = _validate_cooked_contract(
        cooked_document, cooked, shader_map
    )
    _require_dependency_pin(
        abi_document,
        "shaderMap",
        shader_map_document,
        shader_map_identity,
        NAMED_ABI_PATH,
    )
    _require_dependency_pin(
        abi_document,
        "cookedPixelShaders",
        cooked_document,
        cooked_identity,
        NAMED_ABI_PATH,
    )
    _validate_named_abi_contract(
        abi_document, abi, cooked, digest_sizes
    )
    _require_dependency_pin(
        child_parent_document,
        "cookedPixelShaders",
        cooked_document,
        cooked_identity,
        CHILD_PARENT_RESOLUTION_PATH,
    )
    _require_dependency_pin(
        child_parent_document,
        "namedAbi",
        abi_document,
        abi_identity,
        CHILD_PARENT_RESOLUTION_PATH,
    )
    child_parent = _validate_child_parent_contract(
        child_parent_document, shader_map, cooked, abi
    )
    translations = _index_translations(
        translations_document, HLSL_TRANSLATIONS_PATH, digest_sizes
    )
    cooked_shader_identities, translated_shader_identities = (
        _validate_program_artifacts(root, translations, digest_sizes)
    )

    occurrences, per_document, loaded, authored_identities = _collect_source_rows(
        root, document_names
    )

    per_parent: Dict[str, List[Dict[str, Any]]] = {}
    for occurrence in occurrences:
        parent, resolved_by, receipt_row_sha = _resolve_parent_material_path(
            occurrence["authoredParentMaterialPath"],
            occurrence["sourceMaterialPath"],
            child_parent,
        )
        status, blocker, _ = _classify(
            parent,
            occurrence["sourceMaterialPath"],
            occurrence["carrier"],
            resolved_by,
            shader_map.get(parent),
            cooked.get(parent),
            abi.get(parent),
            translations,
        )
        occurrence["parentMaterialPath"] = parent
        occurrence["parentResolution"] = resolved_by
        occurrence["childParentRowSha256"] = receipt_row_sha
        occurrence["status"] = status
        occurrence["blocker"] = blocker
        per_parent.setdefault(parent, []).append(occurrence)

    rows = [
        _build_row(
            parent,
            parent_occurrences,
            shader_map.get(parent),
            cooked.get(parent),
            abi.get(parent),
            translations,
        )
        for parent, parent_occurrences in per_parent.items()
    ]
    # Sorting by descending occurrence keeps the highest-leverage family first and
    # makes the artifact byte-stable across runs.
    rows.sort(key=lambda row: (-row["occurrenceCount"], row["parentMaterialPath"]))

    status_family_coverage_counts = {status: 0 for status in ALL_STATUSES}
    status_occurrence_counts = {status: 0 for status in ALL_STATUSES}
    for row in rows:
        for evidence_class in row["evidenceClasses"]:
            status = evidence_class["status"]
            status_family_coverage_counts[status] += 1
            status_occurrence_counts[status] += evidence_class["occurrenceCount"]

    document_summaries = []
    for name in loaded:
        document_occurrences = [
            occurrence for occurrence in occurrences if occurrence["document"] == name
        ]
        exact_program = sum(
            occurrence["status"]
            in (
                STATUS_PROGRAM_EXACT_NAMED_ONLY,
                STATUS_PROGRAM_EXACT_NAMING_MISSING,
            )
            for occurrence in document_occurrences
        )
        pending_program = sum(
            occurrence["status"]
            in (
                STATUS_PROGRAM_PENDING_NAMED_ONLY,
                STATUS_PROGRAM_PENDING_NAMING_MISSING,
            )
            for occurrence in document_occurrences
        )
        exact_named_only = sum(
            occurrence["status"] == STATUS_PROGRAM_EXACT_NAMED_ONLY
            for occurrence in document_occurrences
        )
        representative_named_only = sum(
            occurrence["status"] == STATUS_PROGRAM_PENDING_NAMED_ONLY
            for occurrence in document_occurrences
        )
        document_summaries.append(
            {
                "document": name,
                "sourceRowCount": per_document[name]["sourceRowCount"],
                "executionRowCount": per_document[name]["executionRowCount"],
                "programExactRowCount": exact_program,
                "programPermutationPendingRowCount": pending_program,
                "exactNamedMappingOnlyRowCount": exact_named_only,
                "representativeNamedMappingOnlyRowCount": representative_named_only,
                "runtimeAbiClosedRowCount": 0,
                "productAdmittedRowCount": 0,
            }
        )

    artifact: Dict[str, Any] = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "identity": {
            "scope": "AUTHORED_SOURCE_ROW_TO_MATERIAL_FAMILY_SHADER_EVIDENCE",
            "joinKey": "material.sourceMaterialPath -> pinned child-parent receipt; "
            "otherwise material.sourceProfile.parentMaterialPath",
            "joinPolicy": "EXACT_PATH_ONLY_NO_LEAF_FALLBACK",
            "admits": "OCCURRENCE_PROGRAM_ONLY_WHEN_CHILD_PERMUTATION_AND_CARRIER_MATCH; "
            "NAMED_MAPPING_EVIDENCE_ONLY; NO_RUNTIME_ABI_OR_PRODUCT",
            "note": (
                "A parent-family representative program is not occurrence exact. "
                "Exact requires SINGLE_PERMUTATION_FAMILY or the selected child "
                "MIC plus the same carrier. Named lanes still do not close source "
                "values, sampler state, VF/pass/MRT, dispatch, composition, or Product."
            ),
        },
        "inputs": {
            "shaderMapIndex": SHADER_MAP_INDEX_PATH,
            "shaderMapIndexArtifactSha256": shader_map_document["artifactSha256"],
            "shaderMapIndexRawSha256": shader_map_identity["rawSha256"],
            "shaderMapIndexByteSize": shader_map_identity["byteSize"],
            "cookedPixelShaders": COOKED_PIXEL_SHADERS_PATH,
            "cookedPixelShadersArtifactSha256": cooked_document["artifactSha256"],
            "cookedPixelShadersRawSha256": cooked_identity["rawSha256"],
            "cookedPixelShadersByteSize": cooked_identity["byteSize"],
            "hlslTranslations": HLSL_TRANSLATIONS_PATH,
            "hlslTranslationsRawSha256": translations_identity["rawSha256"],
            "hlslTranslationsByteSize": translations_identity["byteSize"],
            "hlslTranslationProgramCount": len(translations),
            "cookedShaderDirectory": COOKED_SHADER_DIRECTORY,
            "cookedShaderProgramCount": len(cooked_shader_identities),
            "cookedShaderSetSha256": _canonical_sha256(
                cooked_shader_identities
            ),
            "translatedShaderDirectory": TRANSLATED_SHADER_DIRECTORY,
            "translatedShaderFileCount": len(translated_shader_identities),
            "translatedShaderSetSha256": _canonical_sha256(
                translated_shader_identities
            ),
            "namedAbi": NAMED_ABI_PATH,
            "namedAbiArtifactSha256": abi_document["artifactSha256"],
            "namedAbiRawSha256": abi_identity["rawSha256"],
            "namedAbiByteSize": abi_identity["byteSize"],
            "childParentResolution": CHILD_PARENT_RESOLUTION_PATH,
            "childParentResolutionArtifactSha256": child_parent_document[
                "artifactSha256"
            ],
            "childParentResolutionRawSha256": child_parent_identity["rawSha256"],
            "childParentResolutionByteSize": child_parent_identity["byteSize"],
            "authoredDocuments": list(loaded),
            "authoredDocumentIdentities": authored_identities,
        },
        "summary": {
            "familyCount": len(rows),
            "occurrenceCount": sum(row["occurrenceCount"] for row in rows),
            "statusFamilyCoverageCounts": status_family_coverage_counts,
            "statusOccurrenceCounts": status_occurrence_counts,
            "parentResolvedByChildReceiptOccurrenceCount": sum(
                occurrence["parentResolution"]
                in (
                    "CHILD_PARENT_KNOWN_FAMILY_EXACT",
                    "CHILD_PARENT_NEW_FAMILY_EXACT",
                )
                for occurrence in occurrences
            ),
            "documents": document_summaries,
        },
        "families": rows,
    }
    artifact["artifactSha256"] = _canonical_sha256(artifact)
    return artifact


def serialize_inventory(artifact: Dict[str, Any]) -> bytes:
    """Return the one canonical on-disk representation of an inventory.

    The artifact is hash-bearing evidence, so newline conversion is not a
    presentation detail: a CRLF checkout would make its raw identity differ
    from the bytes produced by the publisher.  Keeping serialization in one
    function also makes ``--check`` compare against exactly what publish writes.
    """

    return (
        json.dumps(artifact, ensure_ascii=False, indent=1) + "\n"
    ).encode("utf-8")


def write_inventory(root: str, artifact: Dict[str, Any]) -> str:
    absolute_path = os.path.join(root, OUTPUT_PATH)
    os.makedirs(os.path.dirname(absolute_path), exist_ok=True)
    temporary_path = absolute_path + ".tmp"
    with open(temporary_path, "wb") as handle:
        handle.write(serialize_inventory(artifact))
    os.replace(temporary_path, absolute_path)
    return absolute_path


def check_inventory(root: str, artifact: Dict[str, Any]) -> str:
    """Fail unless the published artifact is byte-identical to current inputs."""

    absolute_path = os.path.join(root, OUTPUT_PATH)
    if not os.path.isfile(absolute_path):
        raise InventoryError("published contract is absent: " + OUTPUT_PATH)
    with open(absolute_path, "rb") as handle:
        actual = handle.read()
    expected = serialize_inventory(artifact)
    if actual != expected:
        raise InventoryError(
            "published contract is stale: "
            + OUTPUT_PATH
            + " (run without --check to replace it)"
        )
    return absolute_path


def main(argv: Optional[List[str]] = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repository-root", default=REPOSITORY_ROOT)
    parser.add_argument(
        "--check",
        action="store_true",
        help="classify and report without replacing the published contract",
    )
    arguments = parser.parse_args(argv)

    try:
        artifact = build_inventory(arguments.repository_root, TARGET_DOCUMENTS)
    except InventoryError as error:
        sys.stderr.write("FAIL " + str(error) + "\n")
        return 1

    summary = artifact["summary"]
    for entry in summary["documents"]:
        sys.stdout.write(
            "%-52s source=%4d execution=%3d exact=%4d pending=%4d "
            "exactNamed=%4d runtimeAbi=%4d\n"
            % (
                entry["document"],
                entry["sourceRowCount"],
                entry["executionRowCount"],
                entry["programExactRowCount"],
                entry["programPermutationPendingRowCount"],
                entry["exactNamedMappingOnlyRowCount"],
                entry["runtimeAbiClosedRowCount"],
            )
        )
    sys.stdout.write(
        "\n%-58s %14s %12s\n"
        % ("status", "familyCoverage", "occurrences")
    )
    for status in ALL_STATUSES:
        sys.stdout.write(
            "%-58s %14d %12d\n"
            % (
                status,
                summary["statusFamilyCoverageCounts"][status],
                summary["statusOccurrenceCounts"][status],
            )
        )

    if arguments.check:
        try:
            checked = check_inventory(arguments.repository_root, artifact)
        except InventoryError as error:
            sys.stderr.write("FAIL " + str(error) + "\n")
            return 1
        sys.stdout.write(
            "\nCHECK current "
            + os.path.relpath(checked, arguments.repository_root)
            + "\n"
        )
        return 0

    written = write_inventory(arguments.repository_root, artifact)
    sys.stdout.write("\nwrote " + os.path.relpath(written, arguments.repository_root) + "\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
