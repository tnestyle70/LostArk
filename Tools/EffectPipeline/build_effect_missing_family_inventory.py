#!/usr/bin/env python3
"""Enumerate every Effect material family and what each one still needs.

`build_effect_family_manifest.py` answers "which families exist and how large
is each one".  This tool answers the next question: for each family, is there
a typed HLSL executor in the product renderer, and if not, which recovery
input is the first one missing.

Three joins make that answerable without opinion:

* the authored corpus supplies the family denominator and its occurrences;
* `Client/Public/Effect_MaterialTemplate.h` supplies the exact parent-material
  paths the product renderer already gates a typed executor on, parsed from
  the header rather than restated here, so the inventory cannot drift from
  the code it describes;
* the bulk cooked-pixel-shader receipt, the canary variant contract and the
  pinned source-package manifest supply the evidence tiers a new family
  formula needs - an exact DXBC blob to connect to a runtime executor, or at
  minimum the raw package the join would read.

The inventory also counts the authored elements that carry no source parent at
all.  Those are project-authored approximations, not families with a missing
formula, and folding them into a family denominator would misstate how much of
the corpus a family programme can reach.

Read-only with respect to the corpus.  The only file written is the inventory.
"""

from __future__ import annotations

import argparse
import collections
import hashlib
import json
import re
import sys
from pathlib import Path
from typing import Any

sys.path.insert(0, str(Path(__file__).resolve().parent))

from build_effect_family_manifest import (  # noqa: E402
    DEFAULT_EVIDENCE_ROOT,
    carrier_of,
    character_class,
    index_evidence,
    parent_leaf,
    parse_props,
    role_set_of,
)

REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
AUTHORED_DIRECTORY = REPOSITORY_ROOT / "Data/Effects/Authored"
SOURCE_CATALOG = REPOSITORY_ROOT / "Data/Effects/EffectCatalog.json"
MATERIAL_TEMPLATE_HEADER = (
    REPOSITORY_ROOT / "Client/Public/Effect_MaterialTemplate.h")
DOCUMENT_RENDERER_SOURCE = (
    REPOSITORY_ROOT / "Client/Private/Effect_DocumentRenderer.cpp")
COMMON_EFFECT_SHADER = (
    REPOSITORY_ROOT / "Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli")
COOKED_SHADER_VARIANTS = (
    REPOSITORY_ROOT
    / "Data/Effects/Contracts/ue3-exact-cooked-shader-variants.v1.json")
BULK_COOKED_PIXEL_SHADERS = (
    REPOSITORY_ROOT
    / "Data/Effects/Contracts/effect-family-cooked-pixel-shaders.v1.json")
SHADER_FILE_DIRECTORY = REPOSITORY_ROOT / "Client/Bin/ShaderFiles"
DEFAULT_OUTPUT = (
    REPOSITORY_ROOT
    / "Data/Effects/Contracts/effect-missing-family-inventory.v1.json")
DEFAULT_SOURCE_PACK_MANIFEST = Path(
    r"C:\Users\user\Desktop\Resource_LostArk\00_SourcePackages"
    r"\Effect_DIMENSIONMASTER_20260803_v3\source_pack_manifest.json"
)

SCHEMA = "lostark.effect-missing-family-inventory"
FORMAT_VERSION = 1

PROVENANCE_BOUNDED = "BOUNDED_PARENT_PROPS_RECONSTRUCTION"
PROVENANCE_DXBC = "DXBC_REPLAYED_TRANSLATION"
PROVENANCE_UNMAPPED = "EXECUTOR_OPCODE_NOT_MAPPED"

STATUS_IMPLEMENTED = "TYPED_EXECUTOR_IMPLEMENTED"
STATUS_DXBC_READY = "DXBC_JOINED_RUNTIME_EXECUTOR_NOT_IMPLEMENTED"
STATUS_DXBC_BLOCKED = "DXBC_EXTRACTION_BLOCKED"
STATUS_PACKAGE_READY = "SOURCE_PACKAGE_PRESENT_DXBC_NOT_JOINED"
STATUS_PROPS_ONLY = "PARENT_PROPS_ONLY_NO_SOURCE_PACKAGE"
STATUS_NO_EVIDENCE = "NO_PARENT_EVIDENCE"


class InventoryError(RuntimeError):
    pass


def require(condition: bool, message: str) -> None:
    if not condition:
        raise InventoryError(message)


def canonical_json(value: Any) -> str:
    return json.dumps(
        value, sort_keys=True, separators=(",", ":"), ensure_ascii=False
    )


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_json(value).encode("utf-8")).hexdigest()


def read_json(path: Path) -> Any:
    return json.loads(path.read_text(encoding="utf-8"))


def read_required_json(path: Path, label: str) -> Any:
    """Read a required evidence receipt without falling back to no evidence."""
    require(path.is_file(), f"required {label} is absent: {path}")
    try:
        return read_json(path)
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise InventoryError(f"required {label} is corrupt: {path}") from error


def file_sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def parse_typed_executor_registry(header_text: str) -> dict[str, list[str]]:
    """Map parent material path -> typed executor names, read from the header.

    The resolver is a straight-line chain of if/return blocks.  Every
    parent-material string literal seen since the previous return belongs to
    the profile named by the next return, so a single forward pass over the
    comparison and return sites recovers the mapping without re-encoding it.
    """
    body_start = header_text.find(
        "inline EFFECT_STRICT_TYPED_SOURCE_PROFILE "
        "Resolve_EffectStrictTypedSourceProfile(")
    require(body_start >= 0, "typed source profile resolver is absent")
    body_end = header_text.find("\n}", body_start)
    require(body_end > body_start, "typed source profile resolver is unclosed")
    body = header_text[body_start:body_end]

    token = re.compile(
        r"strParentMaterialPath\s*==\s*\n?\s*\"([^\"]+)\""
        r"|return\s+EFFECT_STRICT_TYPED_SOURCE_PROFILE::(\w+);"
    )
    registry: dict[str, list[str]] = collections.defaultdict(list)
    pending: list[str] = []
    for match in token.finditer(body):
        parent, profile = match.group(1), match.group(2)
        if parent is not None:
            pending.append(parent)
            continue
        if profile in ("NONE", "END"):
            pending.clear()
            continue
        for path in pending:
            if profile not in registry[path]:
                registry[path].append(profile)
        pending.clear()
    require(bool(registry), "typed source profile resolver matched no parents")
    return {key: sorted(value) for key, value in sorted(registry.items())}


def parse_executor_opcodes(renderer_text: str) -> dict[str, list[int]]:
    """Map typed executor name -> the family-profile opcodes it dispatches to.

    The renderer selects an effective evaluator in one switch over the typed
    profile.  Reading the opcodes back out of that switch keeps the provenance
    column tied to the code that actually runs.
    """
    opcodes: dict[str, list[int]] = collections.defaultdict(list)
    current: str | None = None
    pattern = re.compile(
        r"case\s+Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::(\w+):"
        r"|return\s+(\d+)u;"
    )
    for match in pattern.finditer(renderer_text):
        profile, opcode = match.group(1), match.group(2)
        if profile is not None:
            current = None if profile in ("NONE", "END") else profile
            continue
        if current is not None:
            value = int(opcode)
            if value not in opcodes[current]:
                opcodes[current].append(value)
    return {key: sorted(value) for key, value in opcodes.items()}


def parse_bounded_profile_opcodes(shader_text: str) -> set[int]:
    """Opcodes whose formula lives in the shared parent-props evaluator."""
    return {
        int(value)
        for value in re.findall(
            r"(\d+)\s*==\s*g_SourceMaterialProfile", shader_text)
    }


def parse_dxbc_translated_families(shader_directory: Path) -> set[str]:
    """Parent materials whose formula came from a replayed DXBC translation.

    The cross-class UE3 family header is the only place a replayed Artist F
    program is re-exposed to other classes, so its documented parents are the
    entire DXBC-translated family set.
    """
    header = shader_directory / "Shader_EffectUe3MaterialFamilies.hlsli"
    if not header.is_file():
        return set()
    text = header.read_text(encoding="utf-8", errors="replace")
    return set(re.findall(r"\b(fx_[a-z0-9_]*_pa_[a-z0-9_]+)\b", text))


def index_canary_cooked_dxbc(path: Path) -> dict[str, dict[str, Any]]:
    """Map parent material path -> detailed canary variant evidence.

    This older contract is optional because the bulk receipt is the family
    denominator.  Its variants are retained as corroborating evidence for the
    five deeply decoded canaries; they are no longer the only DXBC join.
    """
    if not path.is_file():
        return {}
    try:
        document = read_json(path)
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise InventoryError(f"canary cooked-shader contract is corrupt: {path}") \
            from error
    variants = document.get("variants") if isinstance(document, dict) else None
    require(isinstance(variants, list),
            "canary cooked-shader contract has no variants array")
    index: dict[str, dict[str, Any]] = {}
    for variant in variants:
        require(isinstance(variant, dict),
                "canary cooked-shader variant is not an object")
        parent = variant.get("parentMaterialPath")
        require(isinstance(parent, str) and bool(parent),
                "canary cooked-shader variant has no parentMaterialPath")
        require(parent not in index,
                f"duplicate canary parentMaterialPath: {parent}")
        pixel_shader = variant.get("pixelShader") or {}
        admission = variant.get("admission") or {}
        digest = pixel_shader.get("sha256") \
            or (pixel_shader.get("dxbc") or {}).get("sha256")
        require(isinstance(digest, str) and bool(digest),
                f"canary cooked-shader variant has no DXBC identity: {parent}")
        index[parent] = {
            "provenance": "UE3_EXACT_CANARY_VARIANT_RECEIPT",
            "variantId": variant.get("variantId"),
            "dxbcSha256": digest,
            "exactPixelShaderBlob": bool(
                admission.get("exactPixelShaderBlob")),
            "productRuntime": bool(admission.get("productRuntime")),
            "openBlockers": list(variant.get("openBlockers") or []),
        }
    return index


def index_bulk_cooked_dxbc(path: Path) -> dict[str, dict[str, Any]]:
    """Index the required bulk family receipt by exact full parent path.

    A leaf-name or case-folded join would collapse unrelated UE3 objects.  A
    duplicate parent is therefore either a duplicate receipt row or conflicting
    evidence, and both cases abort before an inventory can be published.
    """
    document = read_required_json(path, "bulk cooked-pixel-shader receipt")
    require(isinstance(document, dict),
            "bulk cooked-pixel-shader receipt is not an object")
    require(document.get("schema") == "lostark.effect-family-cooked-pixel-shaders",
            "bulk cooked-pixel-shader receipt has an unexpected schema")
    require(document.get("formatVersion") == 1,
            "bulk cooked-pixel-shader receipt has an unexpected formatVersion")

    artifact_sha256 = document.get("artifactSha256")
    require(isinstance(artifact_sha256, str) and bool(artifact_sha256),
            "bulk cooked-pixel-shader receipt has no artifactSha256")
    identity_document = dict(document)
    del identity_document["artifactSha256"]
    require(canonical_sha256(identity_document) == artifact_sha256,
            "bulk cooked-pixel-shader receipt artifactSha256 does not match")

    identity = document.get("identity")
    require(isinstance(identity, dict)
            and identity.get("admits") == "COOKED_PROGRAM_ONLY",
            "bulk cooked-pixel-shader receipt overclaims its admission")
    families = document.get("families")
    require(isinstance(families, list),
            "bulk cooked-pixel-shader receipt has no families array")

    index: dict[str, dict[str, Any]] = {}
    source_rows: dict[str, dict[str, Any]] = {}
    extracted_count = 0
    extracted_occurrences = 0
    for family in families:
        require(isinstance(family, dict),
                "bulk cooked-pixel-shader family row is not an object")
        parent = family.get("parentMaterialPath")
        require(isinstance(parent, str) and bool(parent),
                "bulk cooked-pixel-shader row has no parentMaterialPath")
        if parent in source_rows:
            if canonical_json(source_rows[parent]) == canonical_json(family):
                raise InventoryError(
                    f"duplicate bulk cooked-pixel-shader parentMaterialPath: {parent}")
            raise InventoryError(
                f"conflicting bulk cooked-pixel-shader parentMaterialPath: {parent}")
        source_rows[parent] = family

        occurrence_count = family.get("occurrenceCount")
        require(isinstance(occurrence_count, int) and occurrence_count >= 0,
                f"bulk cooked-pixel-shader row has invalid occurrenceCount: {parent}")
        status = family.get("status")
        require(status in ("EXTRACTED", "BLOCKED"),
                f"bulk cooked-pixel-shader row has invalid status: {parent}")
        if status == "EXTRACTED":
            digest = family.get("dxbcSha256")
            byte_size = family.get("dxbcByteSize")
            require(isinstance(digest, str)
                    and re.fullmatch(r"[0-9a-f]{64}", digest) is not None,
                    f"extracted bulk row has invalid DXBC identity: {parent}")
            require(isinstance(byte_size, int) and byte_size > 0,
                    f"extracted bulk row has invalid DXBC byte size: {parent}")
            require(family.get("admits") == "COOKED_PROGRAM_ONLY",
                    f"extracted bulk row has invalid admission: {parent}")
            require(not family.get("blocker"),
                    f"extracted bulk row also claims a blocker: {parent}")
            extracted_count += 1
            extracted_occurrences += occurrence_count
            open_blockers: list[str] = []
        else:
            require(isinstance(family.get("blocker"), str)
                    and bool(family.get("blocker")),
                    f"blocked bulk row has no blocker: {parent}")
            require(not family.get("dxbcSha256")
                    and not family.get("dxbcByteSize"),
                    f"blocked bulk row conflicts with extracted evidence: {parent}")
            digest = None
            byte_size = None
            open_blockers = [str(family["blocker"])]

        index[parent] = {
            "provenance": "BULK_FAMILY_COOKED_PIXEL_SHADER_RECEIPT",
            "receiptStatus": status,
            "dxbcSha256": digest,
            "dxbcByteSize": byte_size,
            "carrier": family.get("carrier"),
            "rendererType": family.get("rendererType"),
            "childMaterialPath": family.get("childMaterialPath"),
            "permutationSelection": family.get("permutationSelection"),
            "exactPixelShaderBlob": status == "EXTRACTED",
            "productRuntime": False,
            "openBlockers": open_blockers,
        }

    summary = document.get("summary")
    require(isinstance(summary, dict),
            "bulk cooked-pixel-shader receipt has no summary")
    require(summary.get("familyCount") == len(families),
            "bulk cooked-pixel-shader familyCount does not reconcile")
    require(summary.get("extractedCount") == extracted_count,
            "bulk cooked-pixel-shader extractedCount does not reconcile")
    require(summary.get("blockedCount") == len(families) - extracted_count,
            "bulk cooked-pixel-shader blockedCount does not reconcile")
    require(summary.get("extractedOccurrenceCount") == extracted_occurrences,
            "bulk cooked-pixel-shader extractedOccurrenceCount does not reconcile")
    return index


def merge_cooked_dxbc_evidence(
    bulk: dict[str, dict[str, Any]],
    canaries: dict[str, dict[str, Any]],
) -> dict[str, dict[str, Any]]:
    """Use the bulk family receipt as primary and retain canary corroboration."""
    merged = {parent: dict(row) for parent, row in bulk.items()}
    for parent, canary in canaries.items():
        if parent not in merged:
            merged[parent] = dict(canary)
            continue
        row = merged[parent]
        row["canaryVariant"] = canary
        identities = sorted({
            digest for digest in (row.get("dxbcSha256"),
                                  canary.get("dxbcSha256"))
            if digest
        })
        row["joinedDxbcSha256s"] = identities
    return merged


def index_source_packages(path: Path) -> dict[str, Any] | None:
    """Map logical package name -> staged raw payload identity."""
    if not path.is_file():
        return None
    document = read_json(path)
    packages = document.get("packages")
    require(isinstance(packages, list), "source pack manifest has no packages")
    index: dict[str, Any] = {}
    for package in packages:
        name = str(package.get("logicalPackage") or "").lower()
        if not name or not package.get("resolved"):
            continue
        index.setdefault(name, {
            "physicalPackage": package.get("physicalPackage"),
            "byteSize": package.get("byteSize"),
            "sha256": package.get("sha256"),
        })
    return index


def package_of(parent_material_path: str) -> str | None:
    """The leading segment is the package only when the path is fully qualified.

    UE3 object paths here are package.group.object.  A two-segment path has
    lost its package during intake, and guessing one from the group name would
    invent provenance the corpus does not have.
    """
    segments = parent_material_path.split(".")
    return segments[0].lower() if len(segments) >= 3 else None


def shader_include_index(directory: Path) -> dict[str, str]:
    if not directory.is_dir():
        return {}
    return {
        path.name: file_sha256(path)
        for path in sorted(directory.glob("Shader_Effect*.hlsli"))
    }


def build_inventory(
    repository_root: Path,
    evidence_root: Path,
    source_pack_manifest: Path,
) -> dict[str, Any]:
    catalog = {
        row["effectAssetId"]
        for row in read_json(
            repository_root / SOURCE_CATALOG.relative_to(REPOSITORY_ROOT)
        )["effects"]
    }
    header_path = repository_root / MATERIAL_TEMPLATE_HEADER.relative_to(
        REPOSITORY_ROOT)
    typed_registry = parse_typed_executor_registry(
        header_path.read_text(encoding="utf-8", errors="replace"))
    renderer_path = repository_root / DOCUMENT_RENDERER_SOURCE.relative_to(
        REPOSITORY_ROOT)
    executor_opcodes = parse_executor_opcodes(
        renderer_path.read_text(encoding="utf-8", errors="replace"))
    common_shader_path = repository_root / COMMON_EFFECT_SHADER.relative_to(
        REPOSITORY_ROOT)
    bounded_opcodes = parse_bounded_profile_opcodes(
        common_shader_path.read_text(encoding="utf-8", errors="replace"))
    dxbc_translated = parse_dxbc_translated_families(SHADER_FILE_DIRECTORY)
    bulk_receipt_path = (
        repository_root
        / BULK_COOKED_PIXEL_SHADERS.relative_to(REPOSITORY_ROOT))
    bulk_dxbc_index = index_bulk_cooked_dxbc(bulk_receipt_path)
    canary_dxbc_index = index_canary_cooked_dxbc(
        repository_root / COOKED_SHADER_VARIANTS.relative_to(REPOSITORY_ROOT))
    dxbc_index = merge_cooked_dxbc_evidence(
        bulk_dxbc_index, canary_dxbc_index)
    package_index = index_source_packages(source_pack_manifest)
    evidence_index = index_evidence(evidence_root)
    authored = repository_root / AUTHORED_DIRECTORY.relative_to(
        REPOSITORY_ROOT)

    accumulator: dict[str, dict[str, Any]] = {}
    approximations: dict[str, collections.Counter] = collections.defaultdict(
        collections.Counter)
    document_count = 0
    catalog_document_count = 0
    element_count = 0
    source_element_count = 0
    disabled_source_element_count = 0
    approximation_element_count = 0

    for path in sorted(authored.glob("*.effect.json")):
        document = read_json(path)
        effect_asset_id = document.get("effectAssetId", path.stem)
        document_count += 1
        in_catalog = effect_asset_id in catalog
        if in_catalog:
            catalog_document_count += 1
        klass = character_class(effect_asset_id)
        for element in document.get("elements", []):
            element_count += 1
            material = element.get("material") or {}
            profile = material.get("sourceProfile") or {}
            parent = profile.get("parentMaterialPath") or ""
            if not parent:
                approximation_element_count += 1
                approximations[klass][element.get("kind") or "unknown"] += 1
                continue
            source_element_count += 1
            if not profile.get("enabled"):
                disabled_source_element_count += 1
            family = accumulator.setdefault(parent, {
                "occurrences": 0,
                "enabledOccurrences": 0,
                "catalogOccurrences": 0,
                "classes": collections.Counter(),
                "carriers": collections.Counter(),
                "renderProfiles": collections.Counter(),
                "roleSets": collections.Counter(),
                "childMaterials": set(),
                "profileIds": set(),
                "skills": set(),
            })
            family["occurrences"] += 1
            if profile.get("enabled"):
                family["enabledOccurrences"] += 1
            if in_catalog:
                family["catalogOccurrences"] += 1
            family["classes"][klass] += 1
            family["carriers"][carrier_of(element)] += 1
            family["renderProfiles"][
                material.get("renderProfile") or "(none)"] += 1
            family["roleSets"][role_set_of(element)] += 1
            family["childMaterials"].add(
                material.get("sourceMaterialPath") or "(none)")
            if profile.get("profileId"):
                family["profileIds"].add(profile["profileId"])
            family["skills"].add(effect_asset_id)

    families = []
    for parent, row in accumulator.items():
        leaf = parent_leaf(parent)
        props_path = evidence_index.get(leaf)
        if props_path is None:
            props: dict[str, Any] = {"status": "ABSENT", "objectName": leaf}
        else:
            props = {"status": "PRESENT", "objectName": leaf}
            props.update(parse_props(
                props_path.read_text(encoding="utf-8", errors="replace")))

        package = package_of(parent)
        if package_index is None:
            package_state = "UNKNOWN_MANIFEST_UNAVAILABLE"
            package_identity = None
        elif package is None:
            package_state = "PARENT_PATH_HAS_NO_PACKAGE_SEGMENT"
            package_identity = None
        elif package in package_index:
            package_state = "PRESENT"
            package_identity = package_index[package]
        else:
            package_state = "ABSENT"
            package_identity = None

        typed = typed_registry.get(parent, [])
        dxbc = dxbc_index.get(parent)

        opcodes = sorted({
            opcode
            for executor in typed
            for opcode in executor_opcodes.get(executor, [])
        })
        if not typed:
            provenance = None
        elif parent.rsplit(".", 1)[-1] in dxbc_translated:
            provenance = PROVENANCE_DXBC
        elif any(opcode in bounded_opcodes for opcode in opcodes):
            provenance = PROVENANCE_BOUNDED
        else:
            provenance = PROVENANCE_UNMAPPED

        if typed:
            status = STATUS_IMPLEMENTED
        elif dxbc is not None and dxbc["exactPixelShaderBlob"]:
            status = STATUS_DXBC_READY
        elif dxbc is not None and dxbc.get("receiptStatus") == "BLOCKED":
            status = STATUS_DXBC_BLOCKED
        elif package_state == "PRESENT":
            status = STATUS_PACKAGE_READY
        elif props["status"] == "PRESENT":
            status = STATUS_PROPS_ONLY
        else:
            status = STATUS_NO_EVIDENCE

        blockers: list[str] = []
        if status != STATUS_IMPLEMENTED:
            if dxbc is None:
                blockers.append("EXACT_COOKED_PIXEL_SHADER_NOT_JOINED")
            elif status == STATUS_DXBC_READY:
                blockers.append("TYPED_RUNTIME_EXECUTOR_NOT_IMPLEMENTED")
                blockers.extend(dxbc["openBlockers"])
            elif not dxbc["productRuntime"]:
                blockers.extend(dxbc["openBlockers"])
            if package_state == "ABSENT":
                blockers.append("SOURCE_PACKAGE_NOT_STAGED")
            if package_state == "PARENT_PATH_HAS_NO_PACKAGE_SEGMENT":
                blockers.append("PARENT_PATH_PACKAGE_SEGMENT_LOST_AT_INTAKE")
            if props["status"] != "PRESENT":
                blockers.append("PARENT_MATERIAL_PROPS_ABSENT")
        if len(row["carriers"]) > 1:
            blockers.append("CARRIER_SPLIT_SPRITE_AND_MESH")
        if len(row["roleSets"]) > 1:
            blockers.append("NAMED_ROLE_SET_VARIANCE")
        if len(row["renderProfiles"]) > 1:
            blockers.append("RENDER_PROFILE_VARIANCE")

        families.append({
            "familyId": "family-" + hashlib.sha256(
                parent.encode("utf-8")).hexdigest()[:16],
            "parentMaterialPath": parent,
            "restorationStatus": status,
            "typedExecutors": typed,
            "familyProfileOpcodes": opcodes,
            "formulaProvenance": provenance,
            "cookedShaderEvidenceProvenance": (
                dxbc.get("provenance") if dxbc is not None else None),
            "occurrenceCount": row["occurrences"],
            "enabledOccurrenceCount": row["enabledOccurrences"],
            "catalogOccurrenceCount": row["catalogOccurrences"],
            "skillCount": len(row["skills"]),
            "classes": dict(sorted(row["classes"].items())),
            "carriers": dict(sorted(row["carriers"].items())),
            "renderProfiles": dict(sorted(row["renderProfiles"].items())),
            "roleSets": [
                {"roles": list(roles), "occurrenceCount": count}
                for roles, count in sorted(
                    row["roleSets"].items(), key=lambda kv: (-kv[1], kv[0]))
            ],
            "childMaterialCount": len(row["childMaterials"]),
            "childMaterials": sorted(row["childMaterials"]),
            "profileIds": sorted(row["profileIds"]),
            "sourcePackage": {
                "packageName": package,
                "state": package_state,
                "identity": package_identity,
            },
            "cookedPixelShader": dxbc,
            "parentProps": props,
            "blockers": sorted(set(blockers)),
        })

    families.sort(
        key=lambda item: (-item["occurrenceCount"], item["parentMaterialPath"]))
    for family in families:
        family["rowSha256"] = canonical_sha256(family)

    total_occurrences = sum(item["occurrenceCount"] for item in families)
    implemented = [
        item for item in families
        if item["restorationStatus"] == STATUS_IMPLEMENTED
    ]
    missing = [
        item for item in families
        if item["restorationStatus"] != STATUS_IMPLEMENTED
    ]
    implemented_occurrences = sum(
        item["occurrenceCount"] for item in implemented)
    missing_occurrences = total_occurrences - implemented_occurrences
    provenance_occurrences: collections.Counter = collections.Counter()
    for item in implemented:
        provenance_occurrences[item["formulaProvenance"]] += item[
            "occurrenceCount"]
    cooked_evidence_status: collections.Counter = collections.Counter()
    cooked_evidence_status_occurrences: collections.Counter = \
        collections.Counter()
    cooked_evidence_provenance: collections.Counter = collections.Counter()
    cooked_evidence_provenance_occurrences: collections.Counter = \
        collections.Counter()
    for item in families:
        cooked = item["cookedPixelShader"]
        if cooked is None:
            evidence_status = "EXACT_DXBC_NOT_JOINED"
        elif cooked["exactPixelShaderBlob"]:
            evidence_status = "EXACT_DXBC_JOINED"
        else:
            evidence_status = "DXBC_EXTRACTION_BLOCKED"
        cooked_evidence_status[evidence_status] += 1
        cooked_evidence_status_occurrences[evidence_status] += item[
            "occurrenceCount"]
        evidence_provenance = item["cookedShaderEvidenceProvenance"]
        if evidence_provenance is not None:
            cooked_evidence_provenance[evidence_provenance] += 1
            cooked_evidence_provenance_occurrences[
                evidence_provenance] += item["occurrenceCount"]

    running = 0
    coverage_ranks = []
    for index, item in enumerate(
            sorted(missing, key=lambda row: (
                -row["occurrenceCount"], row["parentMaterialPath"])), start=1):
        running += item["occurrenceCount"]
        coverage_ranks.append({
            "rank": index,
            "parentMaterialPath": item["parentMaterialPath"],
            "restorationStatus": item["restorationStatus"],
            "occurrenceCount": item["occurrenceCount"],
            "cumulativeMissingOccurrences": running,
            "cumulativeMissingCoverage": round(
                running / missing_occurrences, 6)
            if missing_occurrences else 0.0,
        })

    inventory = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "inputs": {
            "authoredDirectory": "Data/Effects/Authored",
            "sourceCatalog": "Data/Effects/EffectCatalog.json",
            "typedExecutorHeader": "Client/Public/Effect_MaterialTemplate.h",
            "typedExecutorHeaderSha256": file_sha256(header_path),
            "cookedShaderVariants": (
                "Data/Effects/Contracts/"
                "ue3-exact-cooked-shader-variants.v1.json"),
            "bulkCookedPixelShaders": (
                "Data/Effects/Contracts/"
                "effect-family-cooked-pixel-shaders.v1.json"),
            "bulkCookedPixelShadersSha256": file_sha256(bulk_receipt_path),
            "parentMaterialPropsIndexed": len(evidence_index),
            "sourcePackManifest": str(source_pack_manifest),
            "sourcePackagesIndexed": (
                len(package_index) if package_index is not None else None),
            "shaderIncludes": shader_include_index(SHADER_FILE_DIRECTORY),
        },
        "summary": {
            "authoredDocumentCount": document_count,
            "catalogAdmittedDocumentCount": catalog_document_count,
            "authoredElementCount": element_count,
            "sourceDerivedElementCount": source_element_count,
            "disabledSourceElementCount": disabled_source_element_count,
            "projectAuthoredApproximationElementCount":
                approximation_element_count,
            "familyCount": len(families),
            "familyOccurrenceCount": total_occurrences,
            "typedExecutorFamilyCount": len(implemented),
            "typedExecutorOccurrenceCount": implemented_occurrences,
            "runtimeExecutorPendingFamilyCount": len(missing),
            "runtimeExecutorPendingOccurrenceCount": missing_occurrences,
            "missingFamilyCount": len(missing),
            "missingOccurrenceCount": missing_occurrences,
            "restorationStatusCounts": dict(sorted(collections.Counter(
                item["restorationStatus"] for item in families).items())),
            "formulaProvenanceCounts": dict(sorted(collections.Counter(
                item["formulaProvenance"] for item in implemented).items())),
            "formulaProvenanceOccurrenceCounts": dict(sorted(
                provenance_occurrences.items())),
            "cookedShaderEvidenceStatusCounts": dict(sorted(
                cooked_evidence_status.items())),
            "cookedShaderEvidenceStatusOccurrenceCounts": dict(sorted(
                cooked_evidence_status_occurrences.items())),
            "cookedShaderEvidenceProvenanceCounts": dict(sorted(
                cooked_evidence_provenance.items())),
            "cookedShaderEvidenceProvenanceOccurrenceCounts": dict(sorted(
                cooked_evidence_provenance_occurrences.items())),
            "blockerCounts": dict(sorted(collections.Counter(
                blocker for item in families
                for blocker in item["blockers"]).items())),
        },
        "projectAuthoredApproximations": {
            klass: dict(sorted(counter.items()))
            for klass, counter in sorted(approximations.items())
        },
        "missingFamilyCoverageByRank": coverage_ranks,
        "families": families,
    }
    inventory["artifactSha256"] = canonical_sha256(inventory)
    return inventory


def write_inventory(path: Path, inventory: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(
        json.dumps(inventory, indent=2, ensure_ascii=False) + "\n",
        encoding="utf-8",
        newline="\n",
    )
    temporary.replace(path)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--evidence-root", type=Path,
                        default=DEFAULT_EVIDENCE_ROOT)
    parser.add_argument("--source-pack-manifest", type=Path,
                        default=DEFAULT_SOURCE_PACK_MANIFEST)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument(
        "--check",
        action="store_true",
        help="Compare deterministic output with the existing inventory.",
    )
    arguments = parser.parse_args(argv)
    try:
        inventory = build_inventory(
            REPOSITORY_ROOT,
            arguments.evidence_root,
            arguments.source_pack_manifest,
        )
    except InventoryError as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1

    if arguments.check:
        if not arguments.output.is_file():
            print(f"FAIL: inventory is missing: {arguments.output}",
                  file=sys.stderr)
            return 1
        if read_json(arguments.output) != inventory:
            print("FAIL: generated inventory differs from the checked-in file",
                  file=sys.stderr)
            return 1
        print(f"PASS: {arguments.output}")
    else:
        write_inventory(arguments.output, inventory)
        print(f"WROTE: {arguments.output}")

    summary = inventory["summary"]
    print(
        "RESULT: "
        f"families={summary['familyCount']} "
        f"implemented={summary['typedExecutorFamilyCount']} "
        f"runtimePending={summary['runtimeExecutorPendingFamilyCount']} "
        "exactDxbcJoined="
        f"{summary['cookedShaderEvidenceStatusCounts']['EXACT_DXBC_JOINED']} "
        "dxbcExtractionBlocked="
        f"{summary['cookedShaderEvidenceStatusCounts']['DXBC_EXTRACTION_BLOCKED']} "
        "exactDxbcNotJoined="
        f"{summary['cookedShaderEvidenceStatusCounts']['EXACT_DXBC_NOT_JOINED']} "
        f"approximations={summary['projectAuthoredApproximationElementCount']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
