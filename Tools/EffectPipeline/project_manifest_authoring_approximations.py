#!/usr/bin/env python3
"""Project manifest-approved authored particle rows into editable Approximate state.

The projector is deliberately data-driven.  It does not know character classes,
skill IDs, animation cues, or Product catalog entries.  A role-composition
manifest supplies the exact authored stable IDs, imported evidence identity,
resource overrides, and the only source modules that may be omitted.

``--capture-contract`` is a one-time evidence pin performed before projection.
It records canonical hashes for the complete imported and authored elements.
``--write`` then changes only the manifest-listed elements and commits every
target document plus the manifest status as one rollback-capable transaction.
``--check`` is read-only and requires the fully projected state.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
from pathlib import Path
import re
import tempfile
from typing import Any, Callable


ROOT = Path(__file__).resolve().parents[2]
DEFAULT_MANIFEST = ROOT / (
    "Data/Effects/AuthoredCorrections/Warlord/"
    "Warlord.user-role-composition.json"
)
PROJECTION_KEY = "authoringApproximateProjection"
EXPECTED_SCHEMA = "lostark.effect-authoring-approximate-projection"
EXPECTED_EXECUTION = {
    "enabled": False,
    "failClosed": True,
    "authoringApproximate": True,
}
SOURCE_NODE_PATTERN = re.compile(
    r"^authored-source-particle:[^|]+\|source:([^|]+)\|element:(.+)$"
)


class ProjectionError(RuntimeError):
    pass


def canonical_json_sha256(value: Any) -> str:
    payload = json.dumps(
        value,
        ensure_ascii=False,
        allow_nan=False,
        sort_keys=True,
        separators=(",", ":"),
    ).encode("utf-8")
    return hashlib.sha256(payload).hexdigest()


def load_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, json.JSONDecodeError) as error:
        raise ProjectionError(f"cannot read JSON '{path}': {error}") from error
    if not isinstance(value, dict):
        raise ProjectionError(f"JSON root is not an object: {path}")
    return value


def safe_repo_path(root: Path, relative: str) -> Path:
    if not isinstance(relative, str) or not relative or "\\" in relative:
        raise ProjectionError(f"invalid repository-relative path: {relative!r}")
    pure = Path(relative)
    if pure.is_absolute() or ":" in relative or any(part == ".." for part in pure.parts):
        raise ProjectionError(f"unsafe repository-relative path: {relative!r}")
    resolved_root = root.resolve()
    resolved = (resolved_root / pure).resolve()
    try:
        resolved.relative_to(resolved_root)
    except ValueError as error:
        raise ProjectionError(f"path escapes repository root: {relative!r}") from error
    return resolved


def element_by_id(document: dict[str, Any], stable_id: str, path: Path) -> dict[str, Any]:
    elements = document.get("elements")
    if not isinstance(elements, list):
        raise ProjectionError(f"document elements are invalid: {path}")
    matches = [row for row in elements if isinstance(row, dict) and row.get("id") == stable_id]
    if len(matches) != 1:
        raise ProjectionError(
            f"expected one element '{stable_id}' in '{path}', found {len(matches)}"
        )
    return matches[0]


def _projection_contract(manifest: dict[str, Any]) -> tuple[dict[str, Any], list[dict[str, Any]], dict[str, dict[str, Any]]]:
    projection = manifest.get(PROJECTION_KEY)
    rows = manifest.get("provisionalApproximatePromotions")
    if not isinstance(projection, dict) or projection.get("schema") != EXPECTED_SCHEMA:
        raise ProjectionError("authoring Approximate projection schema is missing or invalid")
    if projection.get("formatVersion") != 1:
        raise ProjectionError("unsupported authoring Approximate projection version")
    if not isinstance(rows, list) or not isinstance(projection.get("rules"), list):
        raise ProjectionError("projection rows/rules are invalid")
    rules = projection["rules"]
    if len(rows) != projection.get("targetCount") or len(rules) != len(rows):
        raise ProjectionError("projection target denominator is inconsistent")
    row_ids = [row.get("stableId") for row in rows if isinstance(row, dict)]
    rule_ids = [rule.get("stableId") for rule in rules if isinstance(rule, dict)]
    if (
        len(row_ids) != len(rows)
        or len(rule_ids) != len(rules)
        or len(set(row_ids)) != len(row_ids)
        or len(set(rule_ids)) != len(rule_ids)
        or set(row_ids) != set(rule_ids)
    ):
        raise ProjectionError("projection stable ID set is missing, duplicated, or divergent")
    if projection.get("targetExecution") != EXPECTED_EXECUTION:
        raise ProjectionError("projection execution contract drifted")
    return projection, rows, {rule["stableId"]: rule for rule in rules}


def _source_identity(element: dict[str, Any]) -> tuple[str, str]:
    source_node = element.get("sourceNode")
    match = SOURCE_NODE_PATTERN.fullmatch(source_node if isinstance(source_node, str) else "")
    if match is None:
        raise ProjectionError(f"cannot resolve imported source identity: {element.get('id', '')}")
    return match.group(1), match.group(2)


def _find_imported_document(root: Path, source_effect_id: str) -> tuple[Path, dict[str, Any]]:
    filename = f"{source_effect_id}.effect.json"
    candidates = sorted((root / "Data/Effects/Imported").rglob(filename))
    matches: list[tuple[Path, dict[str, Any]]] = []
    for path in candidates:
        document = load_json(path)
        if document.get("effectAssetId") == source_effect_id:
            matches.append((path, document))
    if len(matches) != 1:
        raise ProjectionError(
            f"expected one imported document for '{source_effect_id}', found {len(matches)}"
        )
    return matches[0]


def _module_identity(module: dict[str, Any]) -> tuple[str, str]:
    stable_id = module.get("stableId")
    class_name = module.get("className")
    if not isinstance(stable_id, str) or not stable_id or not isinstance(class_name, str) or not class_name:
        raise ProjectionError("source module identity is incomplete")
    return stable_id, class_name


def _validate_rule_against_before(element: dict[str, Any], row: dict[str, Any], rule: dict[str, Any]) -> None:
    if element.get("visible") is not False:
        raise ProjectionError(f"capture input is already visible: {element.get('id', '')}")
    resources = element.get("resources")
    if resources != row.get("resources"):
        raise ProjectionError(f"manifest resource evidence drifted: {element.get('id', '')}")
    material = element.get("material")
    source_profile = material.get("sourceProfile") if isinstance(material, dict) else None
    source_recipe = element.get("sourceRecipe")
    if not isinstance(source_profile, dict) or not isinstance(source_recipe, dict):
        raise ProjectionError(f"source profile/recipe is missing: {element.get('id', '')}")
    if source_profile.get("runtimeShaderProfileId") != row.get("runtimeShaderProfileId"):
        raise ProjectionError(f"manifest pre-profile drifted: {element.get('id', '')}")
    profile_before = rule.get("sourceProfileBefore", {})
    if not isinstance(profile_before, dict) or any(key not in {"subUVMode"} for key in profile_before):
        raise ProjectionError(f"source profile before-contract is invalid: {element.get('id', '')}")
    for key, value in profile_before.items():
        if source_profile.get(key) != value:
            raise ProjectionError(f"source profile before-contract drifted: {element.get('id', '')}/{key}")
    modules = source_recipe.get("modules")
    if not isinstance(modules, list):
        raise ProjectionError(f"source recipe modules are invalid: {element.get('id', '')}")
    by_identity = {_module_identity(module): module for module in modules}
    omitted = rule.get("omittedModules")
    if not isinstance(omitted, list):
        raise ProjectionError(f"omitted module contract is invalid: {element.get('id', '')}")
    omitted_ids: set[str] = set()
    reasons = rule.get("approximationReasons")
    if not isinstance(reasons, list) or not reasons or any(not isinstance(reason, str) or not reason for reason in reasons):
        raise ProjectionError(f"approximation reason is missing: {element.get('id', '')}")
    for omission in omitted:
        if not isinstance(omission, dict):
            raise ProjectionError("omitted module row is invalid")
        identity = (omission.get("stableId"), omission.get("className"))
        if identity not in by_identity or identity[0] in omitted_ids:
            raise ProjectionError(f"omitted module identity drifted: {element.get('id', '')}/{identity}")
        if omission.get("typedReason") not in reasons:
            raise ProjectionError(f"omitted module reason is not explicit: {element.get('id', '')}")
        omitted_ids.add(identity[0])
    overrides = rule.get("resourceOverrides")
    if not isinstance(overrides, list):
        raise ProjectionError(f"resource override contract is invalid: {element.get('id', '')}")
    resource_assets = {
        resource.get("assetId")
        for resource in resources
        if isinstance(resource, dict) and isinstance(resource.get("assetId"), str)
    }
    resource_slots = [resource.get("slotId") for resource in resources if isinstance(resource, dict)]
    for override in overrides:
        if not isinstance(override, dict) or resource_slots.count(override.get("slotId")) != 1:
            raise ProjectionError(f"resource override slot is not exact: {element.get('id', '')}")
        if override.get("assetEvidence") == "SAME_OCCURRENCE_RESOURCE" and override.get("assetId") not in resource_assets:
            raise ProjectionError(f"resource override lacks same-occurrence evidence: {element.get('id', '')}")
        if not isinstance(override.get("typedReason"), str) or not override.get("typedReason"):
            raise ProjectionError(f"resource override reason is missing: {element.get('id', '')}")


def project_element(before: dict[str, Any], row: dict[str, Any], rule: dict[str, Any]) -> dict[str, Any]:
    _validate_rule_against_before(before, row, rule)
    staged = copy.deepcopy(before)
    staged["visible"] = True
    material = staged["material"]
    profile = material["sourceProfile"]
    profile["runtimeShaderProfileId"] = rule.get("runtimeShaderProfileId")
    profile["productAdmissionStatus"] = "AUTHORING_APPROXIMATE"
    profile_overrides = rule.get("sourceProfileOverrides", {})
    if not isinstance(profile_overrides, dict) or any(key not in {"subUVMode"} for key in profile_overrides):
        raise ProjectionError(f"unsupported source profile override: {staged['id']}")
    profile.update(copy.deepcopy(profile_overrides))
    material["execution"] = copy.deepcopy(EXPECTED_EXECUTION)

    resources = staged["resources"]
    for override in rule.get("resourceOverrides", []):
        target = next(resource for resource in resources if resource.get("slotId") == override["slotId"])
        target["assetId"] = override["assetId"]

    omitted = {
        (omission["stableId"], omission["className"])
        for omission in rule.get("omittedModules", [])
    }
    before_modules = staged["sourceRecipe"]["modules"]
    staged["sourceRecipe"]["modules"] = [
        module for module in before_modules if _module_identity(module) not in omitted
    ]
    if len(before_modules) - len(staged["sourceRecipe"]["modules"]) != len(omitted):
        raise ProjectionError(f"source module omission cardinality drifted: {staged['id']}")
    return staged


def _corpus_sha(rows: list[dict[str, Any]], field: str) -> str:
    return canonical_json_sha256(
        [{"stableId": row["stableId"], field: row[field]} for row in sorted(rows, key=lambda value: value["stableId"])]
    )


def capture_contract(root: Path, manifest_path: Path) -> dict[str, Any]:
    manifest = load_json(manifest_path)
    projection, rows, rules = _projection_contract(manifest)
    if projection.get("status") != "CONTRACT_CAPTURE_PENDING" or projection.get("sourceEvidence"):
        raise ProjectionError("projection evidence contract is already captured")
    evidence_rows: list[dict[str, Any]] = []
    document_cache: dict[Path, dict[str, Any]] = {}
    imported_cache: dict[str, tuple[Path, dict[str, Any]]] = {}
    for row in rows:
        stable_id = row["stableId"]
        path = safe_repo_path(root, row["effectDocument"])
        document = document_cache.setdefault(path, load_json(path))
        element = element_by_id(document, stable_id, path)
        rule = rules[stable_id]
        _validate_rule_against_before(element, row, rule)
        source_effect_id, imported_element_id = _source_identity(element)
        imported_path, imported_document = imported_cache.setdefault(
            source_effect_id, _find_imported_document(root, source_effect_id)
        )
        imported_element = element_by_id(imported_document, imported_element_id, imported_path)
        projected = project_element(element, row, rule)
        evidence_rows.append(
            {
                "stableId": stable_id,
                "effectDocument": row["effectDocument"],
                "importedDocument": imported_path.relative_to(root).as_posix(),
                "importedElementId": imported_element_id,
                "authoredBeforeElementCanonicalSha256": canonical_json_sha256(element),
                "authoredBeforeSourceRecipeCanonicalSha256": canonical_json_sha256(element["sourceRecipe"]),
                "authoredBeforeResourcesCanonicalSha256": canonical_json_sha256(element["resources"]),
                "authoredBeforeModulesCanonicalSha256": canonical_json_sha256(element["sourceRecipe"]["modules"]),
                "importedElementCanonicalSha256": canonical_json_sha256(imported_element),
                "importedSourceRecipeCanonicalSha256": canonical_json_sha256(imported_element["sourceRecipe"]),
                "importedResourcesCanonicalSha256": canonical_json_sha256(imported_element["resources"]),
                "importedModulesCanonicalSha256": canonical_json_sha256(imported_element["sourceRecipe"]["modules"]),
                "projectedElementCanonicalSha256": canonical_json_sha256(projected),
                "projectedSourceRecipeCanonicalSha256": canonical_json_sha256(projected["sourceRecipe"]),
                "projectedResourcesCanonicalSha256": canonical_json_sha256(projected["resources"]),
                "projectedModulesCanonicalSha256": canonical_json_sha256(projected["sourceRecipe"]["modules"]),
            }
        )
    evidence = {
        "hashAlgorithm": "canonical-json-sha256-v1",
        "rows": evidence_rows,
        "authoredBeforeElementCorpusSha256": _corpus_sha(evidence_rows, "authoredBeforeElementCanonicalSha256"),
        "authoredBeforeSourceRecipeCorpusSha256": _corpus_sha(evidence_rows, "authoredBeforeSourceRecipeCanonicalSha256"),
        "importedElementCorpusSha256": _corpus_sha(evidence_rows, "importedElementCanonicalSha256"),
        "importedSourceRecipeCorpusSha256": _corpus_sha(evidence_rows, "importedSourceRecipeCanonicalSha256"),
        "projectedElementCorpusSha256": _corpus_sha(evidence_rows, "projectedElementCanonicalSha256"),
        "projectedSourceRecipeCorpusSha256": _corpus_sha(evidence_rows, "projectedSourceRecipeCanonicalSha256"),
    }
    staged_manifest = copy.deepcopy(manifest)
    staged_manifest[PROJECTION_KEY]["sourceEvidence"] = evidence
    staged_manifest[PROJECTION_KEY]["status"] = "CONTRACT_CAPTURED_PENDING_WRITE"
    write_payloads_transactionally({manifest_path: encode_json_like(manifest_path.read_bytes(), staged_manifest)})
    return {"capturedTargetCount": len(evidence_rows), "capturedDocumentCount": len(document_cache)}


def _validate_imported_evidence(root: Path, evidence: dict[str, Any]) -> None:
    cache: dict[Path, dict[str, Any]] = {}
    for pinned in evidence.get("rows", []):
        path = safe_repo_path(root, pinned["importedDocument"])
        document = cache.setdefault(path, load_json(path))
        element = element_by_id(document, pinned["importedElementId"], path)
        checks = {
            "importedElementCanonicalSha256": element,
            "importedSourceRecipeCanonicalSha256": element.get("sourceRecipe"),
            "importedResourcesCanonicalSha256": element.get("resources"),
            "importedModulesCanonicalSha256": element.get("sourceRecipe", {}).get("modules"),
        }
        for key, value in checks.items():
            if canonical_json_sha256(value) != pinned.get(key):
                raise ProjectionError(f"imported source evidence drifted: {pinned['stableId']}/{key}")


def build_projection(root: Path, manifest_path: Path, require_projected: bool) -> tuple[dict[Path, bytes], dict[str, Any]]:
    manifest = load_json(manifest_path)
    projection, rows, rules = _projection_contract(manifest)
    evidence = projection.get("sourceEvidence")
    if not isinstance(evidence, dict) or evidence.get("hashAlgorithm") != "canonical-json-sha256-v1":
        raise ProjectionError("projection source evidence has not been captured")
    pinned_rows = evidence.get("rows")
    if not isinstance(pinned_rows, list) or len(pinned_rows) != len(rows):
        raise ProjectionError("projection evidence denominator drifted")
    pinned_by_id = {row["stableId"]: row for row in pinned_rows}
    if set(pinned_by_id) != set(rules):
        raise ProjectionError("projection evidence stable ID set drifted")
    _validate_imported_evidence(root, evidence)

    documents: dict[Path, dict[str, Any]] = {}
    original_bytes: dict[Path, bytes] = {}
    pre_count = 0
    post_count = 0
    for row in rows:
        stable_id = row["stableId"]
        path = safe_repo_path(root, row["effectDocument"])
        if path not in documents:
            original_bytes[path] = path.read_bytes()
            documents[path] = load_json(path)
        element = element_by_id(documents[path], stable_id, path)
        pinned = pinned_by_id[stable_id]
        current_hash = canonical_json_sha256(element)
        if current_hash == pinned["projectedElementCanonicalSha256"]:
            post_count += 1
            continue
        if current_hash != pinned["authoredBeforeElementCanonicalSha256"]:
            raise ProjectionError(f"authored projection input drifted: {stable_id}")
        if require_projected:
            raise ProjectionError(f"authoring Approximate projection is stale: {stable_id}")
        projected = project_element(element, row, rules[stable_id])
        if canonical_json_sha256(projected) != pinned["projectedElementCanonicalSha256"]:
            raise ProjectionError(f"projected output hash drifted: {stable_id}")
        elements = documents[path]["elements"]
        elements[elements.index(element)] = projected
        pre_count += 1

    if len(documents) != projection.get("targetDocumentCount"):
        raise ProjectionError("projection target document denominator drifted")
    if require_projected:
        if projection.get("status") != "PROJECTED" or any(row.get("status") != "AUTHORING_APPROXIMATE_PROJECTED" for row in rows):
            raise ProjectionError("projection manifest status is stale")
        return {}, {"projectedTargetCount": post_count, "changedTargetCount": 0, "targetDocumentCount": len(documents)}

    staged_manifest = copy.deepcopy(manifest)
    staged_projection = staged_manifest[PROJECTION_KEY]
    staged_projection["status"] = "PROJECTED"
    staged_projection["projectedApproximateCount"] = len(rows)
    for row in staged_manifest["provisionalApproximatePromotions"]:
        row["status"] = "AUTHORING_APPROXIMATE_PROJECTED"
    payloads = {
        path: encode_json_like(original_bytes[path], document)
        for path, document in documents.items()
        if encode_json_like(original_bytes[path], document) != original_bytes[path]
    }
    manifest_payload = encode_json_like(manifest_path.read_bytes(), staged_manifest)
    if manifest_payload != manifest_path.read_bytes():
        payloads[manifest_path] = manifest_payload
    return payloads, {
        "projectedTargetCount": pre_count + post_count,
        "changedTargetCount": pre_count,
        "targetDocumentCount": len(documents),
    }


def encode_json_like(original: bytes, value: dict[str, Any]) -> bytes:
    bom = original.startswith(b"\xef\xbb\xbf")
    newline = "\r\n" if b"\r\n" in original else "\n"
    text = json.dumps(value, ensure_ascii=False, allow_nan=False, indent=2) + "\n"
    if newline != "\n":
        text = text.replace("\n", newline)
    return (b"\xef\xbb\xbf" if bom else b"") + text.encode("utf-8")


def _write_temp(path: Path, payload: bytes) -> Path:
    path.parent.mkdir(parents=True, exist_ok=True)
    handle = tempfile.NamedTemporaryFile(
        mode="wb", delete=False, dir=path.parent, prefix=f".{path.name}.", suffix=".tmp"
    )
    try:
        handle.write(payload)
        handle.flush()
        os.fsync(handle.fileno())
        return Path(handle.name)
    finally:
        handle.close()


def write_payloads_transactionally(
    payloads: dict[Path, bytes],
    replace_func: Callable[[str | os.PathLike[str], str | os.PathLike[str]], None] | None = None,
) -> None:
    if not payloads:
        return
    replace = replace_func or os.replace
    originals = {path: path.read_bytes() if path.exists() else None for path in payloads}
    staged = {path: _write_temp(path, payload) for path, payload in payloads.items()}
    committed: list[Path] = []
    try:
        for path in sorted(staged, key=lambda value: value.as_posix()):
            replace(staged[path], path)
            committed.append(path)
    except Exception:
        restore_errors: list[Exception] = []
        for path in reversed(committed):
            try:
                original = originals[path]
                if original is None:
                    path.unlink(missing_ok=True)
                else:
                    restore = _write_temp(path, original)
                    try:
                        replace(restore, path)
                    finally:
                        restore.unlink(missing_ok=True)
            except Exception as error:  # pragma: no cover - catastrophic I/O path
                restore_errors.append(error)
        if restore_errors:
            raise ProjectionError(f"projection commit and rollback both failed: {restore_errors[0]}")
        raise
    finally:
        for path in staged.values():
            path.unlink(missing_ok=True)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST)
    parser.add_argument("--root", type=Path, default=ROOT)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--capture-contract", action="store_true")
    mode.add_argument("--write", action="store_true")
    mode.add_argument("--check", action="store_true")
    args = parser.parse_args()
    root = args.root.resolve()
    manifest_path = args.manifest.resolve()
    if args.capture_contract:
        result = capture_contract(root, manifest_path)
    else:
        payloads, result = build_projection(root, manifest_path, require_projected=args.check)
        if args.write:
            write_payloads_transactionally(payloads)
    print(json.dumps(result, ensure_ascii=False, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
