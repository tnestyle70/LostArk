#!/usr/bin/env python3
"""Admit and project exact UE3 material families into Product-authored Effects.

The projector is intentionally narrower than the parent-material census.  A
matching parent is only a candidate; promotion additionally requires the exact
profile identity, particle/sprite carrier, render state, source material
identity, and complete required named texture-role closure recorded by the
family registry.  It never publishes the runtime Effect catalog.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import os
import re
import sys
import tempfile
from collections import Counter
from pathlib import Path
from typing import Any


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
REGISTRY_RELATIVE_PATH = (
    "Data/Effects/Contracts/ue3-material-family-registry.v1.json")
RECEIPT_RELATIVE_PATH = (
    "Data/Effects/AuthoredCorrections/Generated/"
    "Ue3MaterialFamilyProjection.receipt.json")
REGISTRY_SCHEMA = "lostark.effect-ue3-material-family-registry"
RECEIPT_SCHEMA = "lostark.effect-ue3-material-family-projection-receipt"


def read_json(path: Path) -> Any:
    return json.loads(path.read_text(encoding="utf-8"))


def canonical_json(value: Any) -> str:
    return json.dumps(
        value, sort_keys=True, separators=(",", ":"), ensure_ascii=False)


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_json(value).encode("utf-8")).hexdigest()


def file_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def validate_registry(registry: dict[str, Any]) -> None:
    if registry.get("schema") != REGISTRY_SCHEMA:
        raise ValueError("unexpected UE3 material-family registry schema")
    if registry.get("formatVersion") != 1:
        raise ValueError("unsupported UE3 material-family registry version")

    seen: dict[str, set[Any]] = {
        "familyId": set(), "runtimeShaderProfileId": set(),
        "nativeProfileIndex": set(), "profileId": set(),
        "parentMaterialPath": set(),
    }
    for family in registry.get("families", []):
        for field, values in seen.items():
            value = family.get(field)
            if value in (None, ""):
                raise ValueError(f"family is missing {field}")
            if value in values:
                raise ValueError(f"duplicate family {field}: {value}")
            values.add(value)
        carrier = family.get("carrier") or {}
        if not carrier.get("elementKind") or not carrier.get("rendererShape"):
            raise ValueError(f"family carrier is incomplete: {family['familyId']}")
        if not family.get("renderProfile"):
            raise ValueError(f"family renderProfile is missing: {family['familyId']}")
        roles = family.get("textureRoles") or []
        lanes = [role.get("lane") for role in roles]
        if lanes != list(range(len(roles))):
            raise ValueError(
                f"texture lanes must be contiguous: {family['familyId']}")
        names = [role.get("name") for role in roles]
        if any(not name for name in names) or len(names) != len(set(names)):
            raise ValueError(
                f"texture role names must be unique: {family['familyId']}")
        if not any(bool(role.get("required")) for role in roles):
            raise ValueError(
                f"family needs at least one required texture: {family['familyId']}")

    corrections = registry.get("targetedCorrections") or []
    correction_keys: set[tuple[str, str, str]] = set()
    for row in corrections:
        key = (row.get("effectAssetId"), row.get("elementId"), row.get("path"))
        if any(not value for value in key) or key in correction_keys:
            raise ValueError(f"invalid or duplicate targeted correction: {key}")
        correction_keys.add(key)


def index_material_identities(contract: dict[str, Any]) -> dict[tuple[str, str], dict]:
    index: dict[tuple[str, str], dict] = {}
    for identity in contract.get("materialIdentities", []):
        key = (
            str(identity.get("sourceMaterialPath") or "").lower(),
            str(identity.get("profileId") or ""),
        )
        if not all(key):
            continue
        if key in index:
            raise ValueError(f"duplicate source material identity: {key}")
        index[key] = identity
    return index


def index_runtime_textures(
    repository_root: Path, registry: dict[str, Any]
) -> dict[str, list[dict]]:
    index: dict[str, list[dict]] = {}
    resources_root = repository_root / "Client/Bin/Resources"
    for runtime_root in registry.get("evidence", {}).get(
            "runtimeTextureRoots", []):
        physical_root = resources_root / runtime_root
        if not physical_root.is_dir():
            continue
        for path in physical_root.rglob("*.dds"):
            relative = path.relative_to(resources_root).as_posix()
            logical_package = path.parent.name.lower()
            leaf = path.stem.lower()
            row = {
                "sourceObjectPath": f"{logical_package}.{leaf}",
                "assetId": relative,
            }
            index.setdefault(leaf, []).append(row)
    return index


def resolve_runtime_texture(
    source_object_path: str, texture_index: dict[str, list[dict]]
) -> dict | None:
    source = source_object_path.lower()
    leaf = source.rsplit(".", 1)[-1]
    candidates = texture_index.get(leaf, [])
    if "." in source:
        exact = [row for row in candidates
                 if row["sourceObjectPath"] == source]
        if len(exact) == 1:
            return exact[0]
    if len(candidates) == 1:
        return candidates[0]
    return None


def supplement_material_identities(
    repository_root: Path, registry: dict[str, Any],
    index: dict[tuple[str, str], dict],
) -> None:
    """Fill Product child identities omitted by the earlier runtime receipt.

    The raw source-material evidence is the authoritative MIC -> parent and
    named parameter table.  Parent defaults are merged first and child named
    overrides replace them.  Anonymous ``umodel_dependency`` rows remain
    evidence only and are never invented as formal family lanes.
    """
    texture_index = index_runtime_textures(repository_root, registry)
    profile_by_parent = {
        family["parentMaterialPath"]: family["profileId"]
        for family in registry["families"]
    }
    for relative in registry.get("evidence", {}).get(
            "sourceMaterialEvidencePaths", []):
        path = repository_root / relative
        if not path.is_file():
            continue
        evidence = read_json(path)
        parents = {
            row.get("parentMaterialPath"): row
            for row in evidence.get("parentMaterialEvidence", {}).values()
        }
        for source_material, instances in evidence.get("materials", {}).items():
            for instance in instances:
                parent = instance.get("parent")
                profile_id = profile_by_parent.get(parent)
                if not profile_id:
                    continue
                key = (source_material.lower(), profile_id)
                if key in index:
                    continue
                parent_evidence = parents.get(parent) or {}
                parent_material = parent_evidence.get("materialEvidence") or {}
                parent_rows = parent_material.get(
                    "collectedTextureParameters") or []
                role_rows: dict[str, dict] = {
                    str(row.get("name")): {
                        "name": str(row.get("name")),
                        "group": str(row.get("group") or ""),
                        "source": str(row.get("texture") or ""),
                    }
                    for row in parent_rows if row.get("name")
                }
                for row in instance.get("textures", []):
                    name = str(row.get("name") or "")
                    if not name or name == "umodel_dependency":
                        continue
                    role_rows[name] = {
                        "name": name,
                        "group": str(role_rows.get(name, {}).get("group") or ""),
                        "source": str(row.get("texture") or ""),
                    }
                textures = []
                for role in role_rows.values():
                    resolved = resolve_runtime_texture(
                        role["source"], texture_index) if role["source"] else None
                    textures.append({
                        "name": role["name"],
                        "group": role["group"],
                        "sourceObjectPath": (
                            resolved["sourceObjectPath"] if resolved else ""),
                        "assetId": resolved["assetId"] if resolved else "",
                        "addressU": "wrap",
                        "addressV": "wrap",
                        "colorSpace": "linear",
                        "samplingEvidence": "legacy_default",
                    })
                index[key] = {
                    "sourceMaterialPath": source_material,
                    "profileId": profile_id,
                    "parentMaterialPath": parent,
                    "sourceParameters": {"textures": textures},
                }


def index_sampling_evidence(
    repository_root: Path, registry: dict[str, Any]
) -> dict[str, dict]:
    index: dict[str, dict] = {}
    for relative in registry.get("evidence", {}).get(
            "textureSamplingEvidencePaths", []):
        path = repository_root / relative
        if not path.is_file():
            continue
        for row in read_json(path).get("textures", []):
            source = str(row.get("sourceObjectPath") or "").lower()
            if not source:
                continue
            existing = index.get(source)
            projection = {
                field: row[field]
                for field in ("addressU", "addressV", "colorSpace",
                              "samplingEvidence")
                if field in row
            }
            if existing is not None and existing != projection:
                raise ValueError(f"conflicting texture sampling evidence: {source}")
            index[source] = projection
    return index


def source_parameter_rows(identity: dict) -> list[dict]:
    return list((identity.get("sourceParameters") or {}).get("textures") or [])


def resolve_role_texture(
    identity: dict, role_name: str, sampling: dict[str, dict]
) -> dict | None:
    candidates = [
        row for row in source_parameter_rows(identity)
        if row.get("name") == role_name
        and row.get("sourceObjectPath") and row.get("assetId")
    ]
    if not candidates:
        return None
    distinct = {
        (row.get("sourceObjectPath"), row.get("assetId"))
        for row in candidates
    }
    if len(distinct) != 1:
        raise ValueError(
            f"ambiguous texture role {role_name} for "
            f"{identity.get('sourceMaterialPath')}")
    source = str(candidates[0]["sourceObjectPath"])
    result = {
        "name": role_name,
        "sourceObjectPath": source,
        "assetId": str(candidates[0]["assetId"]),
        "addressU": str(candidates[0].get("addressU") or "wrap"),
        "addressV": str(candidates[0].get("addressV") or "wrap"),
        "colorSpace": str(candidates[0].get("colorSpace") or "linear"),
        "samplingEvidence": str(
        candidates[0].get("samplingEvidence") or "legacy_default"),
    }
    group = str(candidates[0].get("group") or "")
    if group:
        result["group"] = group
    result.update(sampling.get(source.lower(), {}))
    return result


def project_source_profile(
    profile: dict, family: dict, identity: dict,
    sampling: dict[str, dict], repository_root: Path,
) -> tuple[dict | None, list[str]]:
    blockers: list[str] = []
    projected_textures: list[dict] = []
    for role in family["textureRoles"]:
        texture = resolve_role_texture(identity, role["name"], sampling)
        if texture is None:
            if role["required"]:
                blockers.append(f"REQUIRED_TEXTURE_ROLE_MISSING:{role['name']}")
            continue
        asset_path = repository_root / "Client/Bin/Resources" / texture["assetId"]
        if not asset_path.is_file():
            if role["required"]:
                blockers.append(f"RUNTIME_TEXTURE_MISSING:{role['name']}")
            continue
        projected_textures.append(texture)
    if blockers:
        return None, blockers

    projected = copy.deepcopy(profile)
    projected["runtimeShaderProfileId"] = family["runtimeShaderProfileId"]
    projected["textures"] = projected_textures
    return projected, []


def admit_occurrence(
    element: dict, family: dict, identities: dict[tuple[str, str], dict],
    sampling: dict[str, dict], repository_root: Path,
) -> tuple[dict | None, list[str]]:
    blockers: list[str] = []
    material = element.get("material") or {}
    profile = material.get("sourceProfile") or {}
    carrier = family["carrier"]
    if not profile.get("enabled"):
        blockers.append("SOURCE_PROFILE_DISABLED")
    if element.get("kind") != carrier["elementKind"]:
        blockers.append("ELEMENT_KIND_MISMATCH")
    if (element.get("sourceRecipe") or {}).get(
            "rendererShape") != carrier["rendererShape"]:
        blockers.append("RENDERER_SHAPE_MISMATCH")
    if material.get("renderProfile") != family["renderProfile"]:
        blockers.append("RENDER_PROFILE_MISMATCH")
    if profile.get("profileId") != family["profileId"]:
        blockers.append("PROFILE_ID_MISMATCH")
    if profile.get("parentMaterialPath") != family["parentMaterialPath"]:
        blockers.append("PARENT_MATERIAL_MISMATCH")
    current_runtime_profile = profile.get("runtimeShaderProfileId")
    if current_runtime_profile not in (
            "effect.ue3.grouped-translucent.v1",
            family["runtimeShaderProfileId"]):
        blockers.append("CURRENT_RUNTIME_PROFILE_MISMATCH")

    source_material = str(material.get("sourceMaterialPath") or "")
    identity = identities.get((source_material.lower(), family["profileId"]))
    if identity is None:
        blockers.append("SOURCE_MATERIAL_IDENTITY_MISSING")
    elif identity.get("parentMaterialPath") != family["parentMaterialPath"]:
        blockers.append("EVIDENCE_PARENT_MISMATCH")
    if blockers:
        return None, blockers
    assert identity is not None
    return project_source_profile(
        profile, family, identity, sampling, repository_root)


def find_balanced_object_end(text: str, start: int) -> int:
    if start >= len(text) or text[start] != "{":
        raise ValueError("balanced object scan must start on an opening brace")
    depth = 0
    in_string = False
    escaped = False
    for index in range(start, len(text)):
        char = text[index]
        if in_string:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == '"':
                in_string = False
            continue
        if char == '"':
            in_string = True
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return index + 1
    raise ValueError("unterminated JSON object")


def element_span(text: str, element_id: str) -> tuple[int, int]:
    needle = f'"id": {json.dumps(element_id, ensure_ascii=False)}'
    position = text.find(needle)
    if position < 0 or text.find(needle, position + len(needle)) >= 0:
        raise ValueError(f"element id is missing or duplicated in text: {element_id}")
    start = text.rfind("{", 0, position)
    if start < 0:
        raise ValueError(f"element object start is missing: {element_id}")
    return start, find_balanced_object_end(text, start)


def member_object_span(
    element_text: str, member_name: str
) -> tuple[int, int, str, bool]:
    pattern = re.compile(
        rf'"{re.escape(member_name)}"\s*:\s*\{{')
    matches = list(pattern.finditer(element_text))
    if len(matches) != 1:
        raise ValueError(
            f"expected exactly one {member_name} object, found {len(matches)}")
    start = matches[0].end() - 1
    line_start = element_text.rfind("\n", 0, matches[0].start()) + 1
    line_prefix = element_text[line_start:matches[0].start()]
    indent_match = re.match(r"[ \t]*", line_prefix)
    indent = indent_match.group(0) if indent_match else ""
    inline = bool(line_prefix.strip())
    return start, find_balanced_object_end(element_text, start), indent, inline


def render_json_object(
    value: dict, base_indent: str, newline: str, inline: bool
) -> str:
    if inline:
        compact = json.dumps(value, ensure_ascii=False)
        return "{ " + compact[1:-1] + " }"
    lines = json.dumps(value, indent=2, ensure_ascii=False).splitlines()
    return lines[0] + "".join(newline + base_indent + line for line in lines[1:])


def normalize_optional_texture_groups(profile: dict) -> dict:
    normalized = copy.deepcopy(profile)
    for texture in normalized.get("textures", []):
        if texture.get("group") == "":
            texture.pop("group", None)
    return normalized


def patch_document_text(
    original: str, projections: dict[str, dict],
    size_resets: dict[str, float],
) -> str:
    newline = "\r\n" if "\r\n" in original else "\n"
    replacements: list[tuple[int, int, str]] = []
    for element_id, profile in projections.items():
        element_start, element_end = element_span(original, element_id)
        element_text = original[element_start:element_end]
        member_start, member_end, indent, inline = member_object_span(
            element_text, "sourceProfile")
        existing = json.loads(element_text[member_start:member_end])
        if (normalize_optional_texture_groups(existing) !=
                normalize_optional_texture_groups(profile)):
            replacements.append((
                element_start + member_start,
                element_start + member_end,
                render_json_object(profile, indent, newline, inline),
            ))

    number_pattern = re.compile(
        r'("size"\s*:\s*)([-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?)')
    for element_id, value in size_resets.items():
        element_start, element_end = element_span(original, element_id)
        element_text = original[element_start:element_end]
        scale_start, scale_end, _, _ = member_object_span(
            element_text, "sourceScale")
        scale_text = element_text[scale_start:scale_end]
        matches = list(number_pattern.finditer(scale_text))
        if len(matches) != 1:
            raise ValueError(
                f"expected one sourceScale.size for {element_id}, found {len(matches)}")
        current = float(matches[0].group(2))
        if not math.isclose(current, value, rel_tol=0.0, abs_tol=1e-8):
            replacements.append((
                element_start + scale_start + matches[0].start(2),
                element_start + scale_start + matches[0].end(2),
                json.dumps(value),
            ))

    result = original
    for start, end, replacement in sorted(replacements, reverse=True):
        result = result[:start] + replacement + result[end:]
    return result


def normalize_allowed_changes(
    document: dict, projected_ids: set[str], reset_ids: set[str]
) -> dict:
    normalized = copy.deepcopy(document)
    for element in normalized.get("elements", []):
        element_id = element.get("id")
        if element_id in projected_ids:
            profile = (element.get("material") or {}).get("sourceProfile") or {}
            profile["runtimeShaderProfileId"] = "<projected>"
            profile["textures"] = "<projected>"
        if element_id in reset_ids:
            particle = (element.get("detail") or {}).get("particle") or {}
            scale = particle.get("sourceScale") or {}
            scale["size"] = "<targeted-correction>"
    return normalized


def assert_preserved(
    before: dict, after: dict, projected_ids: set[str], reset_ids: set[str]
) -> None:
    before_elements = before.get("elements", [])
    after_elements = after.get("elements", [])
    before_ids = [row.get("id") for row in before_elements]
    after_ids = [row.get("id") for row in after_elements]
    if before_ids != after_ids:
        raise ValueError("element cardinality, order, or stable IDs changed")
    if normalize_allowed_changes(
            before, projected_ids, reset_ids) != normalize_allowed_changes(
                after, projected_ids, reset_ids):
        raise ValueError("projection changed data outside its owned fields")


def atomic_write_text(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    descriptor, temporary_name = tempfile.mkstemp(
        prefix=path.name + ".", suffix=".tmp", dir=path.parent)
    try:
        with os.fdopen(descriptor, "w", encoding="utf-8", newline="") as stream:
            stream.write(text)
        os.replace(temporary_name, path)
    except Exception:
        try:
            os.unlink(temporary_name)
        except FileNotFoundError:
            pass
        raise


def build_plan(repository_root: Path) -> dict[str, Any]:
    registry_path = repository_root / REGISTRY_RELATIVE_PATH
    registry = read_json(registry_path)
    validate_registry(registry)
    family_by_parent = {
        family["parentMaterialPath"]: family
        for family in registry["families"]
    }
    evidence_path = repository_root / registry["evidence"][
        "sourceMaterialContractPath"]
    identities = index_material_identities(read_json(evidence_path))
    supplement_material_identities(repository_root, registry, identities)
    sampling = index_sampling_evidence(repository_root, registry)

    product = registry["productScope"]
    catalog = read_json(repository_root / product["catalogPath"])
    entries = [
        row for row in catalog.get("effects", [])
        if row.get("payloadKind") == product["payloadKind"]
    ]
    entries.sort(key=lambda row: row["effectAssetId"])

    documents: dict[Path, dict] = {}
    occurrences: list[dict] = []
    rejections: list[dict] = []
    element_count = 0
    for entry in entries:
        path = repository_root / "Data" / entry["authoringPath"]
        original_text = path.read_text(encoding="utf-8")
        document = json.loads(original_text)
        if document.get("effectAssetId") != entry["effectAssetId"]:
            raise ValueError(f"catalog/document identity mismatch: {path}")
        record = {
            "entry": entry,
            "path": path,
            "text": original_text,
            "document": document,
            "projections": {},
            "sizeResets": {},
        }
        documents[path] = record
        for element in document.get("elements", []):
            element_count += 1
            profile = ((element.get("material") or {}).get("sourceProfile") or {})
            family = family_by_parent.get(profile.get("parentMaterialPath"))
            if family is None:
                continue
            projected, blockers = admit_occurrence(
                element, family, identities, sampling, repository_root)
            base = {
                "effectAssetId": entry["effectAssetId"],
                "authoringPath": "Data/" + entry["authoringPath"],
                "elementId": element.get("id"),
                "familyId": family["familyId"],
                "parentMaterialPath": family["parentMaterialPath"],
                "profileId": family["profileId"],
                "runtimeShaderProfileId": family["runtimeShaderProfileId"],
                "sourceMaterialPath": (element.get("material") or {}).get(
                    "sourceMaterialPath"),
            }
            if blockers:
                rejections.append({**base, "blockers": sorted(blockers)})
                continue
            assert projected is not None
            record["projections"][element["id"]] = projected
            occurrences.append({
                **base,
                "textureRoles": [row["name"] for row in projected["textures"]],
                "textureBindings": [
                    {key: row[key] for key in (
                        "name", "sourceObjectPath", "assetId", "addressU",
                        "addressV", "colorSpace", "samplingEvidence")}
                    for row in projected["textures"]
                ],
                "sourceProfileSha256": canonical_sha256(projected),
            })

    corrections: list[dict] = []
    record_by_effect = {
        row["entry"]["effectAssetId"]: row for row in documents.values()
    }
    for correction in registry.get("targetedCorrections", []):
        record = record_by_effect.get(correction["effectAssetId"])
        status = "TARGET_MISSING"
        current: float | None = None
        if record is not None:
            matches = [
                row for row in record["document"].get("elements", [])
                if row.get("id") == correction["elementId"]
            ]
            if len(matches) == 1 and correction["path"] == (
                    "detail.particle.sourceScale.size"):
                current = (((matches[0].get("detail") or {}).get(
                    "particle") or {}).get("sourceScale") or {}).get("size")
                if isinstance(current, (int, float)) and (
                    math.isclose(current, correction["expectedBefore"],
                                 rel_tol=0.0, abs_tol=1e-8)
                    or math.isclose(current, correction["value"],
                                    rel_tol=0.0, abs_tol=1e-8)
                ):
                    record["sizeResets"][correction["elementId"]] = correction[
                        "value"]
                    status = "CURRENT" if math.isclose(
                        current, correction["value"], rel_tol=0.0,
                        abs_tol=1e-8) else "PENDING"
                else:
                    status = "UNEXPECTED_CURRENT_VALUE"
        corrections.append({
            "effectAssetId": correction["effectAssetId"],
            "elementId": correction["elementId"],
            "path": correction["path"],
            "expectedBefore": correction["expectedBefore"],
            "targetValue": correction["value"],
            "currentValue": current,
            "status": status,
        })

    pending_documents = 0
    pending_occurrences = 0
    for record in documents.values():
        projected = patch_document_text(
            record["text"], record["projections"], record["sizeResets"])
        projected_document = json.loads(projected)
        assert_preserved(
            record["document"], projected_document,
            set(record["projections"]), set(record["sizeResets"]))
        record["projectedText"] = projected
        if projected != record["text"]:
            pending_documents += 1
            before_by_id = {
                row["id"]: row for row in record["document"].get("elements", [])
            }
            after_by_id = {
                row["id"]: row for row in projected_document.get("elements", [])
            }
            pending_occurrences += sum(
                before_by_id[element_id] != after_by_id[element_id]
                for element_id in set(record["projections"]) | set(
                    record["sizeResets"])
            )

    return {
        "registry": registry,
        "registryPath": registry_path,
        "evidencePath": evidence_path,
        "documents": documents,
        "documentCount": len(entries),
        "elementCount": element_count,
        "occurrences": sorted(
            occurrences,
            key=lambda row: (row["effectAssetId"], row["elementId"])),
        "rejections": sorted(
            rejections,
            key=lambda row: (row["effectAssetId"], row["elementId"])),
        "corrections": corrections,
        "pendingDocumentCount": pending_documents,
        "pendingOccurrenceCount": pending_occurrences,
    }


def build_receipt(plan: dict[str, Any]) -> dict[str, Any]:
    family_counts = Counter(row["familyId"] for row in plan["occurrences"])
    family_rows = []
    by_id = {row["familyId"]: row for row in plan["registry"]["families"]}
    for family_id in sorted(by_id):
        family = by_id[family_id]
        family_rows.append({
            "familyId": family_id,
            "runtimeShaderProfileId": family["runtimeShaderProfileId"],
            "nativeProfileIndex": family["nativeProfileIndex"],
            "parentMaterialPath": family["parentMaterialPath"],
            "admittedOccurrenceCount": family_counts[family_id],
            "requiredTextureRoles": [
                role["name"] for role in family["textureRoles"]
                if role["required"]
            ],
            "optionalTextureRoles": [
                role["name"] for role in family["textureRoles"]
                if not role["required"]
            ],
        })
    registry_evidence = plan["registry"]["evidence"]
    additional_evidence = []
    for category in (
            "sourceMaterialEvidencePaths", "textureSamplingEvidencePaths"):
        for relative in registry_evidence.get(category, []):
            path = plan["registryPath"].parents[3] / relative
            if path.is_file():
                additional_evidence.append({
                    "kind": category,
                    "path": relative,
                    "sha256": file_sha256(path),
                })
    receipt = {
        "schema": RECEIPT_SCHEMA,
        "formatVersion": 1,
        "inputs": {
            "registryPath": REGISTRY_RELATIVE_PATH,
            "registrySha256": file_sha256(plan["registryPath"]),
            "sourceMaterialContractPath": str(
                plan["registry"]["evidence"][
                    "sourceMaterialContractPath"]),
            "sourceMaterialContractSha256": file_sha256(plan["evidencePath"]),
            "additionalEvidence": additional_evidence,
            "catalogPayloadKind": plan["registry"]["productScope"][
                "payloadKind"],
        },
        "summary": {
            "productDocumentCount": plan["documentCount"],
            "productElementCount": plan["elementCount"],
            "candidateOccurrenceCount": (
                len(plan["occurrences"]) + len(plan["rejections"])),
            "admittedOccurrenceCount": len(plan["occurrences"]),
            "rejectedOccurrenceCount": len(plan["rejections"]),
        },
        "families": family_rows,
        "targetedCorrections": [
            {key: row[key] for key in (
                "effectAssetId", "elementId", "path", "targetValue")}
            for row in plan["corrections"]
        ],
        "occurrences": plan["occurrences"],
        "rejections": plan["rejections"],
    }
    receipt["artifactSha256"] = canonical_sha256(receipt)
    return receipt


def write_projection(plan: dict[str, Any]) -> list[str]:
    changed: list[str] = []
    for path, record in plan["documents"].items():
        if record["projectedText"] == record["text"]:
            continue
        atomic_write_text(path, record["projectedText"])
        changed.append(path.relative_to(REPOSITORY_ROOT).as_posix())
    return changed


def receipt_text(receipt: dict[str, Any]) -> str:
    return json.dumps(receipt, indent=2, ensure_ascii=False) + "\n"


def invalid_correction_statuses(plan: dict[str, Any]) -> list[dict]:
    return [
        row for row in plan["corrections"]
        if row["status"] not in ("CURRENT", "PENDING")
    ]


def print_summary(plan: dict[str, Any]) -> None:
    counts = Counter(row["familyId"] for row in plan["occurrences"])
    print(f"Product direct documents : {plan['documentCount']}")
    print(f"Product elements         : {plan['elementCount']}")
    print(f"Family candidates        : "
          f"{len(plan['occurrences']) + len(plan['rejections'])}")
    print(f"Admitted                 : {len(plan['occurrences'])}")
    print(f"Rejected                 : {len(plan['rejections'])}")
    print(f"Pending documents        : {plan['pendingDocumentCount']}")
    print(f"Pending occurrences      : {plan['pendingOccurrenceCount']}")
    for family_id, count in sorted(counts.items()):
        print(f"  {family_id:<22} {count}")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--mode", choices=("census", "check", "write"), default="check")
    parser.add_argument("--repository-root", type=Path, default=REPOSITORY_ROOT)
    arguments = parser.parse_args(argv)
    repository_root = arguments.repository_root.resolve()

    try:
        plan = build_plan(repository_root)
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    print_summary(plan)
    invalid = invalid_correction_statuses(plan)
    if plan["rejections"] or invalid:
        for row in plan["rejections"]:
            print(f"REJECT {row['effectAssetId']} {row['elementId']}: "
                  f"{', '.join(row['blockers'])}")
        for row in invalid:
            print(f"CORRECTION {row['effectAssetId']} {row['elementId']}: "
                  f"{row['status']}")
        return 1
    if arguments.mode == "census":
        return 0

    receipt_path = repository_root / RECEIPT_RELATIVE_PATH
    if arguments.mode == "check":
        if plan["pendingDocumentCount"]:
            print("FAIL: authored family projection is stale.")
            return 1
        expected = build_receipt(plan)
        if not receipt_path.is_file():
            print(f"FAIL: projection receipt is missing: {receipt_path}")
            return 1
        actual = read_json(receipt_path)
        if actual.get("artifactSha256") != expected["artifactSha256"]:
            print("FAIL: projection receipt is stale.")
            return 1
        print(f"PASS: exact family projection and receipt are current "
              f"({expected['artifactSha256'][:16]}).")
        return 0

    changed = write_projection(plan)
    refreshed = build_plan(repository_root)
    if refreshed["pendingDocumentCount"] or refreshed["rejections"] or (
            invalid_correction_statuses(refreshed)):
        print("FAIL: projection did not converge after write.", file=sys.stderr)
        return 1
    receipt = build_receipt(refreshed)
    atomic_write_text(receipt_path, receipt_text(receipt))
    print(f"WROTE authored documents : {len(changed)}")
    for path in changed:
        print(f"  {path}")
    print(f"WROTE receipt            : "
          f"{receipt_path.relative_to(repository_root).as_posix()}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
