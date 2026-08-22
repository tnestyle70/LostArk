#!/usr/bin/env python3
"""Recover the parent material every authored element lost at intake.

`build_effect_missing_family_inventory.py` splits the authored corpus in two:
elements whose `material.sourceProfile.parentMaterialPath` survived intake, and
elements that carry no parent at all.  The latter population is labelled
"project-authored approximation", which by itself does not prove that no
source family can be recovered.

Most of those elements still carry `material.sourceMaterialPath`, the child
MaterialInstanceConstant the occurrence was authored from.  A MIC serializes
its `Parent` object reference, so the parent this corpus needs is one
tagged-property read away in the same staged source packages the family
pipeline already opens.  Only the join key was lost, not the evidence.

This tool restores that key.  For every distinct orphan child material path it
opens the declaring package, reads the MIC's `Parent`, and walks MIC -> MIC
until a root `Material` export is reached.  The result is the real family
denominator: how many families the corpus actually uses, and how many of them
already have a cooked pixel shader extracted.

A serialized `Parent` reference does not always keep its package segment - UE3
writes the import outer chain it has, so a link can arrive as `group.leaf`.
`build_effect_family_shader_map_index.py` already meets that shape and answers
it by searching every staged package for the leaf; this tool reuses the same
answer, and treats leaves that resolve to disagreeing objects as ambiguous
rather than picking one.

Read-only with respect to the corpus.  Authored documents are not edited here;
patching them is a separate reviewed migration.  The only file written is the
resolution contract.
"""

from __future__ import annotations

import argparse
import collections
import hashlib
import json
import os
import re
import sys
import tempfile
from pathlib import Path
from typing import Any

REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
LEVEL_TOOLS = REPOSITORY_ROOT / "Tools" / "LevelPlacementExtractor"
for extra in (str(Path(__file__).resolve().parent), str(LEVEL_TOOLS)):
    if extra not in sys.path:
        sys.path.insert(0, extra)

from build_effect_family_shader_map_index import (  # noqa: E402
    export_material_rows,
)
from extract_ue3_effect_material_closure import load_package  # noqa: E402
from extract_ue3_placements import (  # noqa: E402
    LOSTARK_KR_AES_KEY,
    package_ref_path,
    parse_tagged_properties,
)

AUTHORED_DIRECTORY = REPOSITORY_ROOT / "Data/Effects/Authored"
COOKED_PIXEL_SHADERS = (
    REPOSITORY_ROOT
    / "Data/Effects/Contracts/effect-family-cooked-pixel-shaders.v1.json")
NAMED_ABI = (
    REPOSITORY_ROOT
    / "Data/Effects/Contracts/effect-family-named-abi.v1.json")
DEFAULT_OUTPUT = (
    REPOSITORY_ROOT
    / "Data/Effects/Contracts/effect-child-parent-resolution.v1.json")
DEFAULT_SOURCE_PACK_MANIFEST = Path(
    r"C:\Users\user\Desktop\Resource_LostArk\00_SourcePackages"
    r"\Effect_DIMENSIONMASTER_20260803_v3\source_pack_manifest.json"
)

SCHEMA = "lostark.effect-child-parent-resolution"
FORMAT_VERSION = 1
COOKED_PIXEL_SHADERS_SCHEMA = "lostark.effect-family-cooked-pixel-shaders"
NAMED_ABI_SCHEMA = "lostark.effect-family-named-abi"
SOURCE_PACK_SCHEMA_VERSION = 1
SHA256_PATTERN = re.compile(r"[0-9a-f]{64}")

# These are deliberate evidence denominators, not convenient observations.
# A canonical regeneration must stop when the corpus or either upstream
# contract changes, so the denominator update is reviewed with the receipt.
EXPECTED_AUTHORED_DOCUMENT_COUNT = 420
EXPECTED_AUTHORED_ELEMENT_COUNT = 7572
EXPECTED_PARENT_RETAINED_ELEMENT_COUNT = 2746
EXPECTED_PARENT_LOST_ELEMENT_COUNT = 4826
EXPECTED_PARENT_LOST_WITH_CHILD_PATH_ELEMENT_COUNT = 4793
EXPECTED_PARENT_LOST_WITHOUT_CHILD_PATH_ELEMENT_COUNT = 33
EXPECTED_DISTINCT_ORPHAN_CHILD_COUNT = 809
EXPECTED_SOURCE_PACKAGE_COUNT = 621
EXPECTED_COOKED_FAMILY_COUNT = 193
EXPECTED_EXTRACTED_FAMILY_COUNT = 180
EXPECTED_NAMED_ABI_FAMILY_COUNT = 180

ROOT_MATERIAL_CLASS = "material"
INSTANCE_MATERIAL_CLASS = "materialinstanceconstant"

STATUS_RESOLVED = "RESOLVED"
STATUS_BLOCKED = "BLOCKED"
STATUS_NAMED_MAPPING_RESOLVED = "RESOLVED_NAMED_MAPPING"

BLOCKER_PACKAGE_NOT_STAGED = "DECLARING_PACKAGE_NOT_STAGED"
BLOCKER_EXPORT_ABSENT = "CHILD_EXPORT_ABSENT_IN_PACKAGE"
BLOCKER_EXPORT_AMBIGUOUS = "CHILD_EXPORT_LEAF_AMBIGUOUS"
BLOCKER_PROPERTY_STREAM = "TAGGED_PROPERTY_STREAM_UNREADABLE"
BLOCKER_PARENT_PROPERTY_ABSENT = "PARENT_OBJECT_PROPERTY_ABSENT"
BLOCKER_CHAIN_UNTERMINATED = "PARENT_CHAIN_DID_NOT_REACH_ROOT_MATERIAL"
BLOCKER_LEAF_ABSENT_IN_PACK = "LEAF_ABSENT_IN_EVERY_STAGED_PACKAGE"
BLOCKER_LEAF_SEARCH_AMBIGUOUS = "LEAF_SEARCH_RESOLVES_TO_DISAGREEING_OBJECTS"

RESOLVED_DECLARED_PACKAGE = "DECLARED_PACKAGE_EXPORT"
RESOLVED_LEAF_SEARCH = "LEAF_NAME_SEARCH"

# A MIC chain in this corpus is at most three links deep.  The cap exists so a
# cyclic or self-referencing Parent fails closed instead of spinning.
MAXIMUM_PARENT_CHAIN_DEPTH = 8

# Each sweep costs one pass over every staged package, so the walker batches
# all leaves that are pending at the same depth into a single sweep.
MAXIMUM_LEAF_SEARCH_SWEEPS = 3


class ResolutionError(RuntimeError):
    pass


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ResolutionError(message)


def canonical_json(value: Any) -> str:
    return json.dumps(
        value, sort_keys=True, separators=(",", ":"), ensure_ascii=False)


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_json(value).encode("utf-8")).hexdigest()


def raw_file_identity(path: Path, description: str) -> dict[str, Any]:
    require(path.is_file(), f"{description} is missing: {path}")
    digest = hashlib.sha256()
    byte_size = 0
    try:
        with path.open("rb") as stream:
            while chunk := stream.read(1024 * 1024):
                digest.update(chunk)
                byte_size += len(chunk)
    except OSError as error:
        raise ResolutionError(
            f"could not read {description}: {path}: {error}") from error
    return {"rawSha256": digest.hexdigest(), "byteSize": byte_size}


def read_json(path: Path, description: str = "JSON document") -> Any:
    try:
        return json.loads(path.read_bytes().decode("utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise ResolutionError(
            f"{description} is not valid UTF-8 JSON: {error}") from error


def read_artifact(
        path: Path,
        schema: str,
        description: str) -> tuple[dict[str, Any], dict[str, Any]]:
    identity = raw_file_identity(path, description)
    document = read_json(path, description)
    require(isinstance(document, dict),
            f"{description} root must be an object")
    require(document.get("schema") == schema,
            f"{description} schema is not supported")
    require(document.get("formatVersion") == FORMAT_VERSION,
            f"{description} formatVersion is not supported")
    artifact_sha = document.get("artifactSha256")
    require(
        isinstance(artifact_sha, str)
        and SHA256_PATTERN.fullmatch(artifact_sha) is not None,
        f"{description} artifactSha256 is missing or malformed",
    )
    unsigned = dict(document)
    unsigned.pop("artifactSha256", None)
    require(canonical_sha256(unsigned) == artifact_sha,
            f"{description} artifactSha256 drifted")
    return document, identity


def require_count(value: Any, description: str) -> int:
    require(isinstance(value, int) and not isinstance(value, bool)
            and value >= 0,
            f"{description} must be a non-negative integer")
    return value


def relative_repository_path(path: Path) -> str:
    try:
        relative = path.resolve().relative_to(REPOSITORY_ROOT.resolve())
    except ValueError:
        return str(path.resolve())
    return relative.as_posix()


def collect_orphan_children(
        authored: Path
        ) -> tuple[dict[str, dict[str, Any]], dict[str, int], dict[str, Any]]:
    """Distinct child material path -> the elements that lost their parent.

    An element counts as an orphan only when it carries no
    `sourceProfile.parentMaterialPath`.  Elements that kept the parent are
    already in the family denominator and are counted separately so the two
    populations stay comparable.
    """
    require(authored.is_dir(), f"authored directory is missing: {authored}")
    document_paths = sorted(authored.glob("*.effect.json"))
    require(bool(document_paths),
            f"authored directory has no effect documents: {authored}")
    orphans: dict[str, dict[str, Any]] = {}
    census: collections.Counter = collections.Counter()
    asset_ids: set[str] = set()
    document_identities: dict[str, dict[str, Any]] = {}
    for document_path in document_paths:
        description = f"authored effect {document_path.name}"
        identity = raw_file_identity(document_path, description)
        document_identities[document_path.name] = identity
        document = read_json(document_path, description)
        require(isinstance(document, dict),
                f"{description} root must be an object")
        asset_id = document.get("effectAssetId")
        require(isinstance(asset_id, str) and bool(asset_id),
                f"{description} effectAssetId is missing")
        require(asset_id not in asset_ids,
                f"authored corpus duplicates effectAssetId: {asset_id}")
        asset_ids.add(asset_id)
        elements = document.get("elements")
        require(isinstance(elements, list),
                f"{description} elements must be an array")
        element_ids: set[str] = set()
        for offset, element in enumerate(elements):
            require(isinstance(element, dict),
                    f"{description} element {offset} must be an object")
            element_id = element.get("id")
            require(isinstance(element_id, str) and bool(element_id),
                    f"{description} element {offset} has no id")
            require(element_id not in element_ids,
                    f"{description} duplicates element id: {element_id}")
            element_ids.add(element_id)
            census["elements"] += 1
            material = element.get("material")
            require(material is None or isinstance(material, dict),
                    f"{description} element {element_id} material is malformed")
            material = material or {}
            profile = material.get("sourceProfile")
            require(profile is None or isinstance(profile, dict),
                    f"{description} element {element_id} sourceProfile is malformed")
            profile = profile or {}
            parent = profile.get("parentMaterialPath")
            require(parent is None or isinstance(parent, str),
                    f"{description} element {element_id} parent path is malformed")
            if parent:
                census["parentRetained"] += 1
                continue
            census["parentLost"] += 1
            child = material.get("sourceMaterialPath")
            require(child is None or isinstance(child, str),
                    f"{description} element {element_id} child path is malformed")
            if not child:
                census["parentLostWithoutChildPath"] += 1
                continue
            census["parentLostWithChildPath"] += 1
            row = orphans.setdefault(child, {
                "childMaterialPath": child,
                "elementCount": 0,
                "effectAssetIds": set(),
            })
            row["elementCount"] += 1
            row["effectAssetIds"].add(asset_id)
    corpus_identity = {
        "documentCount": len(document_paths),
        "documentSetSha256": canonical_sha256(document_identities),
    }
    return orphans, dict(census), corpus_identity


def validate_canonical_corpus_denominators(
        census: dict[str, int],
        corpus_identity: dict[str, Any],
        orphan_count: int) -> None:
    expected = {
        "documentCount": EXPECTED_AUTHORED_DOCUMENT_COUNT,
        "elements": EXPECTED_AUTHORED_ELEMENT_COUNT,
        "parentRetained": EXPECTED_PARENT_RETAINED_ELEMENT_COUNT,
        "parentLost": EXPECTED_PARENT_LOST_ELEMENT_COUNT,
        "parentLostWithChildPath":
            EXPECTED_PARENT_LOST_WITH_CHILD_PATH_ELEMENT_COUNT,
        "parentLostWithoutChildPath":
            EXPECTED_PARENT_LOST_WITHOUT_CHILD_PATH_ELEMENT_COUNT,
        "distinctOrphanChildCount": EXPECTED_DISTINCT_ORPHAN_CHILD_COUNT,
    }
    actual = {
        "documentCount": corpus_identity["documentCount"],
        "elements": census.get("elements", 0),
        "parentRetained": census.get("parentRetained", 0),
        "parentLost": census.get("parentLost", 0),
        "parentLostWithChildPath": census.get(
            "parentLostWithChildPath", 0),
        "parentLostWithoutChildPath": census.get(
            "parentLostWithoutChildPath", 0),
        "distinctOrphanChildCount": orphan_count,
    }
    for key, value in expected.items():
        require(actual[key] == value,
                f"canonical authored denominator {key} must be {value}; "
                f"got {actual[key]}")


def load_validated_package_index(
        manifest_path: Path,
        enforce_canonical_denominator: bool
        ) -> tuple[dict[str, Path], dict[str, Any]]:
    """Parse and authenticate the staged source-pack manifest and packages."""
    manifest_identity = raw_file_identity(
        manifest_path, "source pack manifest")
    manifest = read_json(manifest_path, "source pack manifest")
    require(isinstance(manifest, dict),
            "source pack manifest root must be an object")
    require(manifest.get("schemaVersion") == SOURCE_PACK_SCHEMA_VERSION,
            "source pack manifest schemaVersion is not supported")
    packages = manifest.get("packages")
    require(isinstance(packages, list),
            "source pack manifest packages must be an array")
    if enforce_canonical_denominator:
        require(len(packages) == EXPECTED_SOURCE_PACKAGE_COUNT,
                "canonical source package denominator must be "
                f"{EXPECTED_SOURCE_PACKAGE_COUNT}; got {len(packages)}")

    root = manifest_path.parent.resolve()
    index: dict[str, Path] = {}
    identities: dict[str, dict[str, Any]] = {}
    total_byte_size = 0
    seen_names: set[str] = set()
    seen_paths: set[str] = set()
    for offset, row in enumerate(packages):
        require(isinstance(row, dict),
                f"source package {offset} must be an object")
        logical = row.get("logicalPackage")
        require(isinstance(logical, str) and bool(logical),
                f"source package {offset} logicalPackage is missing")
        key = logical.lower()
        require(key not in seen_names,
                f"source pack duplicates logical package: {logical}")
        seen_names.add(key)
        resolved = row.get("resolved")
        require(isinstance(resolved, bool),
                f"source package {logical} resolved must be boolean")
        if not resolved:
            continue
        relative_text = row.get("relativePath")
        require(isinstance(relative_text, str) and bool(relative_text),
                f"source package {logical} relativePath is missing")
        relative = Path(relative_text)
        require(not relative.is_absolute() and ".." not in relative.parts,
                f"source package {logical} relativePath escapes the pack")
        normalized_relative = relative.as_posix().lower()
        require(normalized_relative not in seen_paths,
                f"source pack duplicates physical path: {relative_text}")
        seen_paths.add(normalized_relative)
        path = (root / relative).resolve()
        try:
            path.relative_to(root)
        except ValueError as error:
            raise ResolutionError(
                f"source package {logical} resolves outside the pack") from error
        expected_sha = row.get("sha256")
        expected_size = row.get("byteSize")
        require(isinstance(expected_sha, str)
                and SHA256_PATTERN.fullmatch(expected_sha) is not None,
                f"source package {logical} sha256 is malformed")
        require_count(expected_size,
                      f"source package {logical} byteSize")
        identity = raw_file_identity(path, f"source package {logical}")
        require(identity["rawSha256"] == expected_sha,
                f"source package {logical} raw SHA-256 drifted")
        require(identity["byteSize"] == expected_size,
                f"source package {logical} byte size drifted")
        index[key] = path
        identities[key] = identity
        total_byte_size += expected_size

    require(bool(index), "source pack manifest resolved no packages")
    summary = manifest.get("summary")
    require(isinstance(summary, dict),
            "source pack manifest summary must be an object")
    require(summary.get("packageCount") == len(packages),
            "source pack manifest summary.packageCount is inconsistent")
    require(summary.get("byteSize") == total_byte_size,
            "source pack manifest summary.byteSize is inconsistent")
    return index, {
        "manifestRawSha256": manifest_identity["rawSha256"],
        "manifestByteSize": manifest_identity["byteSize"],
        "packageCount": len(index),
        "packageByteSize": total_byte_size,
        "packageSetSha256": canonical_sha256(identities),
    }


class PackageCache:
    """Open each staged package at most once.

    Resolving 887 children touches 28 packages; without a cache the same
    hundred-megabyte package is decrypted once per child.
    """

    def __init__(self, package_index: dict[str, Path]) -> None:
        self._index = package_index
        self._packages: dict[str, Any] = {}
        self._rows: dict[str, dict[str, list[dict[str, Any]]]] = {}

    def has(self, package_name: str) -> bool:
        return package_name in self._index

    def rows(self, package_name: str) -> dict[str, list[dict[str, Any]]]:
        if package_name not in self._rows:
            package = self._packages.get(package_name)
            if package is None:
                package = load_package(
                    self._index[package_name], LOSTARK_KR_AES_KEY)
                self._packages[package_name] = package
            self._rows[package_name] = export_material_rows(package)
        return self._rows[package_name]

    def package(self, package_name: str) -> Any:
        self.rows(package_name)
        return self._packages[package_name]

    @property
    def opened_count(self) -> int:
        return len(self._packages)


def parent_reference(package: Any, row: dict[str, Any]) -> str | None:
    """Read the MIC Parent object reference as a package-qualified path."""
    serial = package.logical[
        row["serialOffset"]: row["serialOffset"] + row["serialByteSize"]]
    properties, _ = parse_tagged_properties(
        serial, package.names, package.summary.version)
    entry = properties.get("parent")
    if not isinstance(entry, dict):
        return None
    reference = entry.get("value")
    if not isinstance(reference, int) or reference == 0:
        return None
    return package_ref_path(reference, package.imports, package.exports)


def select_export(
        rows: dict[str, list[dict[str, Any]]],
        leaf: str) -> tuple[dict[str, Any] | None, str | None]:
    """Pick the single Material/MIC export a leaf name refers to.

    A leaf that names more than one Material-class export in one package is
    ambiguous and fails closed; guessing would silently pick another graph.
    """
    candidates = rows.get(leaf.lower()) or []
    if not candidates:
        return None, BLOCKER_EXPORT_ABSENT
    if len(candidates) > 1:
        return None, BLOCKER_EXPORT_AMBIGUOUS
    return candidates[0], None


def canonical_object_path(package_name: str, object_path: str) -> str:
    """Package-qualified identity for one material object.

    Intake preserved the parent path in two shapes - `package.group.leaf` and
    the group-only `group.leaf` - and the current family denominator counts
    those as two families for the same material.  Re-qualifying every
    resolution with the package that actually declares the export collapses
    that split, so the join key is the object rather than the spelling.
    """
    if object_path.lower().startswith(package_name.lower() + "."):
        return object_path
    return f"{package_name}.{object_path}"


class LeafIndex:
    """Leaf object name -> the Material/MIC objects the staged pack declares.

    A `Parent` reference that lost its package segment cannot be opened
    directly, so it is answered by name.  Filling this index costs one pass
    over every staged package, which is why leaves are requested first and
    resolved in batched sweeps rather than one lookup at a time.  A MIC's own
    `Parent` is read while its package is still open so the next chain link
    does not force another sweep.
    """

    def __init__(self, package_index: dict[str, Path]) -> None:
        self._index = package_index
        self._pending: set[str] = set()
        self._entries: dict[str, dict[str, Any]] = {}
        self.sweeps = 0

    def request(self, leaf: str) -> None:
        leaf = leaf.lower()
        if leaf not in self._entries:
            self._pending.add(leaf)

    def has_pending(self) -> bool:
        return bool(self._pending)

    def entry(self, leaf: str) -> dict[str, Any] | None:
        return self._entries.get(leaf.lower())

    def sweep(self, verbose: bool) -> None:
        wanted = set(self._pending)
        self._pending.clear()
        if not wanted:
            return
        self.sweeps += 1
        if verbose:
            print(f"  leaf sweep {self.sweeps}: {len(wanted)} leaves",
                  flush=True)
        found: dict[str, list[dict[str, Any]]] = collections.defaultdict(list)
        for package_name in sorted(self._index):
            package_path = self._index[package_name]
            try:
                package = load_package(
                    package_path, LOSTARK_KR_AES_KEY)
            except Exception as error:  # noqa: BLE001 - preserve source error
                raise ResolutionError(
                    "leaf search package load failed: "
                    f"package={package_name} path={package_path}: "
                    f"{type(error).__name__}: {error}") from error
            try:
                rows = export_material_rows(package)
            except Exception as error:  # noqa: BLE001 - preserve source error
                raise ResolutionError(
                    "leaf search material export scan failed: "
                    f"package={package_name} path={package_path}: "
                    f"{type(error).__name__}: {error}") from error
            for leaf in wanted & set(rows):
                for row in rows[leaf]:
                    candidate = {
                        "packageName": package_name,
                        "className": row["className"],
                        "objectPath": row["objectPath"],
                    }
                    if row["className"] == INSTANCE_MATERIAL_CLASS:
                        try:
                            candidate["parentPath"] = parent_reference(
                                package, row)
                        except Exception:  # noqa: BLE001 - source data
                            candidate["parentPath"] = None
                    found[leaf].append(candidate)
        for leaf in wanted:
            self._entries[leaf] = self._reduce(found.get(leaf) or [])

    @staticmethod
    def _reduce(candidates: list[dict[str, Any]]) -> dict[str, Any]:
        """Accept a leaf only when every hit names the same object."""
        if not candidates:
            return {"blocker": BLOCKER_LEAF_ABSENT_IN_PACK}
        classes = {row["className"] for row in candidates}
        if len(classes) > 1:
            return {"blocker": BLOCKER_LEAF_SEARCH_AMBIGUOUS}
        class_name = classes.pop()
        if class_name == INSTANCE_MATERIAL_CLASS:
            parents = {row.get("parentPath") for row in candidates}
            if len(parents) > 1:
                return {"blocker": BLOCKER_LEAF_SEARCH_AMBIGUOUS}
            parent = parents.pop()
            if not parent:
                return {"blocker": BLOCKER_PARENT_PROPERTY_ABSENT}
            return {
                "className": class_name,
                "parentPath": parent,
                "packageName": candidates[0]["packageName"],
                "candidateCount": len(candidates),
            }
        identities = {
            (row["packageName"].lower(), row["objectPath"].lower())
            for row in candidates
        }
        if len(identities) > 1:
            return {"blocker": BLOCKER_LEAF_SEARCH_AMBIGUOUS}
        return {
            "className": class_name,
            "objectPath": candidates[0]["objectPath"],
            "packageName": candidates[0]["packageName"],
            "candidateCount": len(candidates),
        }


PENDING = "PENDING"


def resolve_child(child_path: str,
                  cache: PackageCache,
                  leaf_index: LeafIndex) -> dict[str, Any]:
    """Walk child -> Parent -> ... -> root Material inside the staged pack.

    Returns a resolved row, a blocked row, or `PENDING` when a chain link can
    only be answered by a leaf sweep that has not run yet.  The caller drives
    the sweeps and calls again.
    """
    chain: list[str] = []
    current = child_path
    for _ in range(MAXIMUM_PARENT_CHAIN_DEPTH):
        segments = current.split(".")
        leaf = segments[-1]
        package_name = segments[0].lower() if len(segments) >= 2 else None

        row = None
        if package_name is not None and cache.has(package_name):
            row, blocker = select_export(cache.rows(package_name), leaf)
            if blocker == BLOCKER_EXPORT_AMBIGUOUS:
                return {
                    "status": STATUS_BLOCKED,
                    "blocker": blocker,
                    "parentChain": chain,
                    "unresolvedPath": current,
                }

        if row is not None:
            resolved_by = RESOLVED_DECLARED_PACKAGE
            class_name = row["className"]
            if class_name == ROOT_MATERIAL_CLASS:
                return {
                    "status": STATUS_RESOLVED,
                    "resolvedBy": resolved_by,
                    "parentChain": chain,
                    "parentMaterialPath": current,
                    "canonicalParentMaterialPath": canonical_object_path(
                        package_name, row["objectPath"]),
                    "parentDeclaringPackage": package_name,
                }
            if class_name != INSTANCE_MATERIAL_CLASS:
                return {
                    "status": STATUS_BLOCKED,
                    "blocker": BLOCKER_EXPORT_ABSENT,
                    "parentChain": chain,
                    "unresolvedPath": current,
                }
            try:
                reference = parent_reference(
                    cache.package(package_name), row)
            except Exception:  # noqa: BLE001 - external source data
                return {
                    "status": STATUS_BLOCKED,
                    "blocker": BLOCKER_PROPERTY_STREAM,
                    "parentChain": chain,
                    "unresolvedPath": current,
                }
            if not reference:
                return {
                    "status": STATUS_BLOCKED,
                    "blocker": BLOCKER_PARENT_PROPERTY_ABSENT,
                    "parentChain": chain,
                    "unresolvedPath": current,
                }
            chain.append(reference)
            current = reference
            continue

        entry = leaf_index.entry(leaf)
        if entry is None:
            leaf_index.request(leaf)
            return {"status": PENDING}
        if "blocker" in entry:
            return {
                "status": STATUS_BLOCKED,
                "blocker": entry["blocker"],
                "parentChain": chain,
                "unresolvedPath": current,
            }
        if entry["className"] == ROOT_MATERIAL_CLASS:
            return {
                "status": STATUS_RESOLVED,
                "resolvedBy": RESOLVED_LEAF_SEARCH,
                "parentChain": chain,
                "parentMaterialPath": current,
                "canonicalParentMaterialPath": canonical_object_path(
                    entry["packageName"], entry["objectPath"]),
                "parentDeclaringPackage": entry["packageName"],
                "leafSearchCandidateCount": entry["candidateCount"],
            }
        chain.append(entry["parentPath"])
        current = entry["parentPath"]
    return {
        "status": STATUS_BLOCKED,
        "blocker": BLOCKER_CHAIN_UNTERMINATED,
        "parentChain": chain,
        "unresolvedPath": current,
    }


def validate_cooked_contract(
        document: dict[str, Any],
        enforce_canonical_denominator: bool) -> dict[str, dict[str, Any]]:
    families = document.get("families")
    require(isinstance(families, list),
            "cooked shader receipt families must be an array")
    by_parent: dict[str, dict[str, Any]] = {}
    extracted = 0
    blockers: collections.Counter[str] = collections.Counter()
    for offset, row in enumerate(families):
        require(isinstance(row, dict),
                f"cooked family {offset} must be an object")
        parent = row.get("parentMaterialPath")
        require(isinstance(parent, str) and bool(parent),
                f"cooked family {offset} has no parentMaterialPath")
        require(parent not in by_parent,
                f"cooked shader receipt duplicates family: {parent}")
        status = row.get("status")
        require(status in ("EXTRACTED", "BLOCKED"),
                f"cooked family {parent} status is not supported")
        if status == "EXTRACTED":
            extracted += 1
            digest = row.get("dxbcSha256")
            require(isinstance(digest, str)
                    and SHA256_PATTERN.fullmatch(digest) is not None,
                    f"cooked family {parent} dxbcSha256 is malformed")
        else:
            blocker = row.get("blocker")
            require(isinstance(blocker, str) and bool(blocker),
                    f"blocked cooked family {parent} has no blocker")
            blocker_key = blocker.split(":", 1)[0].split("(", 1)[0].strip()
            blockers[blocker_key] += 1
        by_parent[parent] = row
    if enforce_canonical_denominator:
        require(len(families) == EXPECTED_COOKED_FAMILY_COUNT,
                "cooked family denominator must be "
                f"{EXPECTED_COOKED_FAMILY_COUNT}; got {len(families)}")
        require(extracted == EXPECTED_EXTRACTED_FAMILY_COUNT,
                "EXTRACTED cooked family denominator must be "
                f"{EXPECTED_EXTRACTED_FAMILY_COUNT}; got {extracted}")
    summary = document.get("summary")
    require(isinstance(summary, dict),
            "cooked shader receipt summary must be an object")
    expected = {
        "familyCount": len(families),
        "extractedCount": extracted,
        "blockedCount": len(families) - extracted,
        "blockerCounts": dict(sorted(blockers.items())),
    }
    for key, value in expected.items():
        require(summary.get(key) == value,
                f"cooked shader receipt summary.{key} is inconsistent")
    return by_parent


def validate_named_abi_contract(
        document: dict[str, Any],
        cooked: dict[str, dict[str, Any]],
        cooked_document: dict[str, Any],
        cooked_identity: dict[str, Any],
        enforce_canonical_denominator: bool) -> dict[str, dict[str, Any]]:
    families = document.get("families")
    require(isinstance(families, list),
            "named ABI receipt families must be an array")
    extracted_parents = {
        parent for parent, row in cooked.items()
        if row["status"] == "EXTRACTED"
    }
    by_parent: dict[str, dict[str, Any]] = {}
    blocked = 0
    blockers: collections.Counter[str] = collections.Counter()
    for offset, row in enumerate(families):
        require(isinstance(row, dict),
                f"named ABI family {offset} must be an object")
        parent = row.get("parentMaterialPath")
        require(isinstance(parent, str) and bool(parent),
                f"named ABI family {offset} has no parentMaterialPath")
        require(parent not in by_parent,
                f"named ABI receipt duplicates family: {parent}")
        require(parent in extracted_parents,
                f"named ABI family is not EXTRACTED upstream: {parent}")
        status = row.get("status")
        require(status in (STATUS_NAMED_MAPPING_RESOLVED, "BLOCKED"),
                f"named ABI family {parent} status is not supported")
        if status == "BLOCKED":
            blocked += 1
            blocker = row.get("blocker")
            require(isinstance(blocker, dict),
                    f"blocked named ABI family {parent} blocker must be "
                    "an object")
            require(set(blocker) == {"reasonCode", "candidateCount"},
                    f"blocked named ABI family {parent} blocker fields "
                    "are malformed")
            reason_code = blocker.get("reasonCode")
            require(isinstance(reason_code, str) and bool(reason_code.strip()),
                    f"blocked named ABI family {parent} reasonCode is "
                    "malformed")
            candidate_count = blocker.get("candidateCount")
            require(isinstance(candidate_count, int)
                    and not isinstance(candidate_count, bool)
                    and candidate_count > 1,
                    f"blocked named ABI family {parent} candidateCount "
                    "must be an integer greater than one")
            blockers[reason_code] += 1
        else:
            digest = row.get("dxbcSha256")
            require(digest == cooked[parent].get("dxbcSha256"),
                    f"named ABI family {parent} DXBC identity drifted")
        by_parent[parent] = row
    require(set(by_parent) == extracted_parents,
            "named ABI family set must equal the EXTRACTED cooked family set")
    if enforce_canonical_denominator:
        require(len(families) == EXPECTED_NAMED_ABI_FAMILY_COUNT,
                "named ABI family denominator must be "
                f"{EXPECTED_NAMED_ABI_FAMILY_COUNT}; got {len(families)}")
    summary = document.get("summary")
    require(isinstance(summary, dict),
            "named ABI receipt summary must be an object")
    expected_summary = {
        "familyCount": len(families),
        "resolvedNamedMappingCount": len(families) - blocked,
        "blockedCount": blocked,
        "blockerCounts": dict(sorted(blockers.items())),
    }
    for key, value in expected_summary.items():
        require(summary.get(key) == value,
                f"named ABI receipt summary.{key} is inconsistent")
    inputs = document.get("inputs")
    require(isinstance(inputs, dict),
            "named ABI receipt inputs must be an object")
    require(inputs.get("cookedPixelShadersArtifactSha256")
            == cooked_document["artifactSha256"],
            "named ABI receipt pins a different cooked artifact")
    require(inputs.get("cookedPixelShadersRawSha256")
            == cooked_identity["rawSha256"],
            "named ABI receipt pins different cooked bytes")
    require(inputs.get("cookedPixelShadersByteSize")
            == cooked_identity["byteSize"],
            "named ABI receipt pins a different cooked byte size")
    return by_parent


class FamilyEvidence:
    """Existing cooked-shader and named-ABI status, addressable by any shape.

    The two contracts are keyed on the corpus spelling of the parent path.  A
    recovered parent is keyed on the package that declares it, so a direct
    lookup would miss whenever the two disagree.  A leaf-name index closes
    that gap, and is used only when the leaf names exactly one known family.
    """

    def __init__(self,
                 cooked_path: Path,
                 named_abi_path: Path,
                 enforce_canonical_denominator: bool) -> None:
        cooked_document, cooked_identity = read_artifact(
            cooked_path, COOKED_PIXEL_SHADERS_SCHEMA,
            "cooked shader receipt")
        cooked_rows = validate_cooked_contract(
            cooked_document, enforce_canonical_denominator)
        named_document, named_identity = read_artifact(
            named_abi_path, NAMED_ABI_SCHEMA, "named ABI receipt")
        named_rows = validate_named_abi_contract(
            named_document, cooked_rows, cooked_document, cooked_identity,
            enforce_canonical_denominator)
        self._cooked = {
            parent: str(row["status"])
            for parent, row in cooked_rows.items()
        }
        self._abi_closed = {
            parent for parent, row in named_rows.items()
            if row.get("status") == STATUS_NAMED_MAPPING_RESOLVED
        }
        self.provenance = {
            "cookedPixelShaders": relative_repository_path(cooked_path),
            "cookedPixelShadersArtifactSha256":
                cooked_document["artifactSha256"],
            "cookedPixelShadersRawSha256": cooked_identity["rawSha256"],
            "cookedPixelShadersByteSize": cooked_identity["byteSize"],
            "namedAbi": relative_repository_path(named_abi_path),
            "namedAbiArtifactSha256": named_document["artifactSha256"],
            "namedAbiRawSha256": named_identity["rawSha256"],
            "namedAbiByteSize": named_identity["byteSize"],
        }
        by_leaf: dict[str, set[str]] = collections.defaultdict(set)
        for path in self._cooked:
            by_leaf[path.rsplit(".", 1)[-1].lower()].add(path)
        self._unique_leaf = {leaf: next(iter(paths))
                             for leaf, paths in by_leaf.items()
                             if len(paths) == 1}

    def key(self, *candidates: str | None) -> str | None:
        """The known family path this parent refers to, or None."""
        for candidate in candidates:
            if candidate and candidate in self._cooked:
                return candidate
        for candidate in candidates:
            if not candidate:
                continue
            match = self._unique_leaf.get(candidate.rsplit(".", 1)[-1].lower())
            if match:
                return match
        return None

    def cooked_status(self, key: str | None) -> str:
        return self._cooked.get(key or "", "ABSENT")

    def abi_closed(self, key: str | None) -> bool:
        return (key or "") in self._abi_closed


def build_resolution(authored: Path,
                     source_pack_manifest: Path,
                     cooked_pixel_shaders: Path,
                     named_abi: Path,
                     enforce_canonical_denominators: bool,
                     verbose: bool) -> dict[str, Any]:
    orphans, census, corpus_identity = collect_orphan_children(authored)
    if enforce_canonical_denominators:
        validate_canonical_corpus_denominators(
            census, corpus_identity, len(orphans))
    package_index, source_pack_identity = load_validated_package_index(
        source_pack_manifest, enforce_canonical_denominators)
    cache = PackageCache(package_index)
    leaf_index = LeafIndex(package_index)
    evidence = FamilyEvidence(
        cooked_pixel_shaders, named_abi,
        enforce_canonical_denominators)

    results: dict[str, dict[str, Any]] = {}
    for _ in range(MAXIMUM_LEAF_SEARCH_SWEEPS + 1):
        for child_path in sorted(orphans):
            if child_path in results:
                continue
            result = resolve_child(child_path, cache, leaf_index)
            if result["status"] != PENDING:
                results[child_path] = result
        if len(results) == len(orphans) or not leaf_index.has_pending():
            break
        leaf_index.sweep(verbose)
    for child_path in sorted(orphans):
        results.setdefault(child_path, {
            "status": STATUS_BLOCKED,
            "blocker": BLOCKER_CHAIN_UNTERMINATED,
            "parentChain": [],
            "unresolvedPath": child_path,
        })

    rows: list[dict[str, Any]] = []
    for child_path in sorted(orphans):
        seed = orphans[child_path]
        result = results[child_path]
        row: dict[str, Any] = {
            "childMaterialPath": child_path,
            "elementCount": seed["elementCount"],
            "effectAssetIdCount": len(seed["effectAssetIds"]),
            "status": result["status"],
        }
        row.update({key: value for key, value in result.items()
                    if key != "status"})
        parent = result.get("canonicalParentMaterialPath")
        if parent:
            key = evidence.key(
                parent, result.get("parentMaterialPath"))
            row["knownFamilyPath"] = key
            row["cookedPixelShaderStatus"] = evidence.cooked_status(key)
            row["namedAbiClosed"] = evidence.abi_closed(key)
            row["familyAlreadyInDenominator"] = key is not None
        row["rowSha256"] = canonical_sha256(row)
        rows.append(row)
        if verbose:
            print(f"{result['status']:<8s} {child_path} -> "
                  f"{parent or result.get('blocker')}", flush=True)

    resolved = [row for row in rows if row["status"] == STATUS_RESOLVED]
    recovered_elements = sum(row["elementCount"] for row in resolved)
    recovered_parents: collections.Counter = collections.Counter()
    known_key: dict[str, str | None] = {}
    for row in resolved:
        parent = row["canonicalParentMaterialPath"]
        recovered_parents[parent] += row["elementCount"]
        known_key[parent] = row.get("knownFamilyPath")

    known = {parent for parent in recovered_parents if known_key.get(parent)}
    new_parents = set(recovered_parents) - known
    extracted_elements = sum(
        count for parent, count in recovered_parents.items()
        if evidence.cooked_status(known_key.get(parent)) == "EXTRACTED")

    families = []
    for parent, count in sorted(
            recovered_parents.items(), key=lambda item: (-item[1], item[0])):
        family = {
            "canonicalParentMaterialPath": parent,
            "knownFamilyPath": known_key.get(parent),
            "recoveredElementCount": count,
            "cookedPixelShaderStatus": evidence.cooked_status(
                known_key.get(parent)),
            "namedAbiClosed": evidence.abi_closed(known_key.get(parent)),
            "alreadyInDenominator": bool(known_key.get(parent)),
        }
        family["rowSha256"] = canonical_sha256(family)
        families.append(family)

    index: dict[str, Any] = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "identity": {
            "scope": "AUTHORED_CORPUS_ORPHAN_CHILD_MATERIALS",
            "admits": "PARENT_MATERIAL_JOIN_KEY_ONLY",
            "note": (
                "A recovered parent restores the family join key an element "
                "lost at intake.  It does not admit a cooked program, a "
                "vertex factory, a sampler state, a runtime binding or a "
                "visual result, and it does not edit the authored document."
            ),
        },
        "inputs": {
            "authoredDirectory": relative_repository_path(authored),
            "authoredDocumentCount": corpus_identity["documentCount"],
            "authoredDocumentSetSha256":
                corpus_identity["documentSetSha256"],
            "sourcePackManifest": str(source_pack_manifest.resolve()),
            "sourcePackManifestRawSha256":
                source_pack_identity["manifestRawSha256"],
            "sourcePackManifestByteSize":
                source_pack_identity["manifestByteSize"],
            "sourcePackageCount": source_pack_identity["packageCount"],
            "sourcePackageByteSize":
                source_pack_identity["packageByteSize"],
            "sourcePackageSetSha256":
                source_pack_identity["packageSetSha256"],
            **evidence.provenance,
            "packagesOpened": cache.opened_count,
            "leafSearchSweeps": leaf_index.sweeps,
        },
        "summary": {
            "authoredElementCount": census.get("elements", 0),
            "parentRetainedElementCount": census.get("parentRetained", 0),
            "parentLostElementCount": census.get("parentLost", 0),
            "parentLostWithChildPathElementCount": census.get(
                "parentLostWithChildPath", 0),
            "parentLostWithoutChildPathElementCount": census.get(
                "parentLostWithoutChildPath", 0),
            "distinctOrphanChildCount": len(rows),
            "resolvedChildCount": len(resolved),
            "resolvedByCounts": dict(sorted(collections.Counter(
                row.get("resolvedBy") for row in resolved).items())),
            "blockedChildCount": len(rows) - len(resolved),
            "recoveredElementCount": recovered_elements,
            "recoveredParentMaterialCount": len(recovered_parents),
            "recoveredParentsAlreadyInDenominator": len(known),
            "recoveredParentsNewToDenominator": len(new_parents),
            "recoveredElementsWithExtractedDxbc": extracted_elements,
            "blockerCounts": dict(sorted(collections.Counter(
                row["blocker"] for row in rows
                if row["status"] == STATUS_BLOCKED).items())),
        },
        "families": families,
        "children": rows,
    }
    index["artifactSha256"] = canonical_sha256(index)
    validate_resolution_contract(
        index,
        enforce_canonical_denominators=enforce_canonical_denominators)
    return index


def require_row_sha256(row: dict[str, Any], description: str) -> None:
    digest = row.get("rowSha256")
    require(isinstance(digest, str)
            and SHA256_PATTERN.fullmatch(digest) is not None,
            f"{description} rowSha256 is missing or malformed")
    unsigned = dict(row)
    unsigned.pop("rowSha256", None)
    require(canonical_sha256(unsigned) == digest,
            f"{description} rowSha256 drifted")


def validate_resolution_contract(
        index: dict[str, Any],
        enforce_canonical_denominators: bool) -> None:
    require(isinstance(index, dict),
            "child-parent receipt root must be an object")
    require(index.get("schema") == SCHEMA,
            "child-parent receipt schema is not supported")
    require(index.get("formatVersion") == FORMAT_VERSION,
            "child-parent receipt formatVersion is not supported")
    artifact_sha = index.get("artifactSha256")
    require(isinstance(artifact_sha, str)
            and SHA256_PATTERN.fullmatch(artifact_sha) is not None,
            "child-parent receipt artifactSha256 is missing or malformed")
    unsigned = dict(index)
    unsigned.pop("artifactSha256", None)
    require(canonical_sha256(unsigned) == artifact_sha,
            "child-parent receipt artifactSha256 drifted")

    identity = index.get("identity")
    require(isinstance(identity, dict),
            "child-parent receipt identity must be an object")
    require(identity.get("scope")
            == "AUTHORED_CORPUS_ORPHAN_CHILD_MATERIALS",
            "child-parent receipt identity.scope is not supported")
    require(identity.get("admits") == "PARENT_MATERIAL_JOIN_KEY_ONLY",
            "child-parent receipt identity.admits is not supported")

    inputs = index.get("inputs")
    require(isinstance(inputs, dict),
            "child-parent receipt inputs must be an object")
    for key in (
            "authoredDocumentSetSha256",
            "sourcePackManifestRawSha256",
            "sourcePackageSetSha256",
            "cookedPixelShadersArtifactSha256",
            "cookedPixelShadersRawSha256",
            "namedAbiArtifactSha256",
            "namedAbiRawSha256"):
        value = inputs.get(key)
        require(isinstance(value, str)
                and SHA256_PATTERN.fullmatch(value) is not None,
                f"child-parent receipt inputs.{key} is malformed")
    for key in (
            "authoredDocumentCount",
            "sourcePackManifestByteSize",
            "sourcePackageCount",
            "sourcePackageByteSize",
            "cookedPixelShadersByteSize",
            "namedAbiByteSize",
            "packagesOpened",
            "leafSearchSweeps"):
        require_count(inputs.get(key),
                      f"child-parent receipt inputs.{key}")
    require(isinstance(inputs.get("authoredDirectory"), str)
            and bool(inputs["authoredDirectory"]),
            "child-parent receipt authoredDirectory is missing")
    require(isinstance(inputs.get("sourcePackManifest"), str)
            and bool(inputs["sourcePackManifest"]),
            "child-parent receipt sourcePackManifest is missing")
    for key in ("cookedPixelShaders", "namedAbi"):
        require(isinstance(inputs.get(key), str) and bool(inputs[key]),
                f"child-parent receipt inputs.{key} is missing")
    require(inputs["sourcePackageCount"] > 0
            and inputs["sourcePackageByteSize"] > 0,
            "child-parent receipt source package identity is empty")
    require(inputs["packagesOpened"] <= inputs["sourcePackageCount"],
            "child-parent receipt packagesOpened exceeds the package set")
    require(inputs["leafSearchSweeps"] <= MAXIMUM_LEAF_SEARCH_SWEEPS,
            "child-parent receipt leafSearchSweeps exceeds the policy cap")
    if enforce_canonical_denominators:
        require(inputs["authoredDocumentCount"]
                == EXPECTED_AUTHORED_DOCUMENT_COUNT,
                "child-parent receipt authored document denominator drifted")
        require(inputs["sourcePackageCount"]
                == EXPECTED_SOURCE_PACKAGE_COUNT,
                "child-parent receipt source package denominator drifted")

    children = index.get("children")
    require(isinstance(children, list),
            "child-parent receipt children must be an array")
    child_paths: set[str] = set()
    resolved_rows: list[dict[str, Any]] = []
    resolved_by_counts: collections.Counter[str] = collections.Counter()
    blocker_counts: collections.Counter[str] = collections.Counter()
    recovered_parent_counts: collections.Counter[str] = collections.Counter()
    recovered_parent_evidence: dict[str, tuple[Any, ...]] = {}
    orphan_element_count = 0
    for offset, row in enumerate(children):
        require(isinstance(row, dict),
                f"child-parent child {offset} must be an object")
        child = row.get("childMaterialPath")
        require(isinstance(child, str) and bool(child),
                f"child-parent child {offset} has no childMaterialPath")
        require(child not in child_paths,
                f"child-parent receipt duplicates child: {child}")
        child_paths.add(child)
        require_row_sha256(row, f"child-parent child {child}")
        element_count = require_count(
            row.get("elementCount"), f"child {child} elementCount")
        asset_count = require_count(
            row.get("effectAssetIdCount"),
            f"child {child} effectAssetIdCount")
        require(element_count > 0 and 0 < asset_count <= element_count,
                f"child {child} occurrence counts are inconsistent")
        orphan_element_count += element_count
        status = row.get("status")
        require(status in (STATUS_RESOLVED, STATUS_BLOCKED),
                f"child {child} status is not supported")
        if status == STATUS_BLOCKED:
            blocker = row.get("blocker")
            require(isinstance(blocker, str) and bool(blocker),
                    f"blocked child {child} has no blocker")
            blocker_counts[blocker] += 1
            continue
        resolved_rows.append(row)
        parent_chain = row.get("parentChain")
        require(isinstance(parent_chain, list)
                and all(isinstance(path, str) and bool(path)
                        for path in parent_chain),
                f"resolved child {child} parentChain is malformed")
        resolved_by = row.get("resolvedBy")
        require(resolved_by in (
            RESOLVED_DECLARED_PACKAGE, RESOLVED_LEAF_SEARCH),
            f"resolved child {child} has unsupported resolution evidence")
        resolved_by_counts[resolved_by] += 1
        parent = row.get("canonicalParentMaterialPath")
        require(isinstance(parent, str) and bool(parent),
                f"resolved child {child} has no canonical parent")
        known = row.get("knownFamilyPath")
        require(known is None or (isinstance(known, str) and bool(known)),
                f"resolved child {child} known family is malformed")
        already = row.get("familyAlreadyInDenominator")
        require(isinstance(already, bool) and already == (known is not None),
                f"resolved child {child} denominator status is inconsistent")
        cooked_status = row.get("cookedPixelShaderStatus")
        require(cooked_status in ("EXTRACTED", "BLOCKED", "ABSENT"),
                f"resolved child {child} cooked status is unsupported")
        named_closed = row.get("namedAbiClosed")
        require(isinstance(named_closed, bool),
                f"resolved child {child} namedAbiClosed must be boolean")
        if known is None:
            require(cooked_status == "ABSENT" and not named_closed,
                    f"new family child {child} carries upstream evidence")
        if named_closed:
            require(cooked_status == "EXTRACTED",
                    f"child {child} closes ABI without an extracted program")
        recovered_parent_counts[parent] += element_count
        evidence = (known, cooked_status, named_closed, already)
        previous = recovered_parent_evidence.setdefault(parent, evidence)
        require(previous == evidence,
                f"children disagree on recovered family evidence: {parent}")

    families = index.get("families")
    require(isinstance(families, list),
            "child-parent receipt families must be an array")
    by_parent: dict[str, dict[str, Any]] = {}
    for offset, row in enumerate(families):
        require(isinstance(row, dict),
                f"child-parent family {offset} must be an object")
        parent = row.get("canonicalParentMaterialPath")
        require(isinstance(parent, str) and bool(parent),
                f"child-parent family {offset} has no canonical parent")
        require(parent not in by_parent,
                f"child-parent receipt duplicates family: {parent}")
        require_row_sha256(row, f"child-parent family {parent}")
        by_parent[parent] = row
    require(set(by_parent) == set(recovered_parent_counts),
            "child-parent family set differs from resolved child parents")
    for parent, row in by_parent.items():
        require(row.get("recoveredElementCount")
                == recovered_parent_counts[parent],
                f"family {parent} recoveredElementCount is inconsistent")
        expected_evidence = recovered_parent_evidence[parent]
        actual_evidence = (
            row.get("knownFamilyPath"),
            row.get("cookedPixelShaderStatus"),
            row.get("namedAbiClosed"),
            row.get("alreadyInDenominator"),
        )
        require(actual_evidence == expected_evidence,
                f"family {parent} evidence differs from its children")

    summary = index.get("summary")
    require(isinstance(summary, dict),
            "child-parent receipt summary must be an object")
    parent_lost_without_child = require_count(
        summary.get("parentLostWithoutChildPathElementCount"),
        "summary.parentLostWithoutChildPathElementCount")
    parent_retained = require_count(
        summary.get("parentRetainedElementCount"),
        "summary.parentRetainedElementCount")
    parent_lost = orphan_element_count + parent_lost_without_child
    known_parents = [
        row for row in families if row["alreadyInDenominator"]]
    extracted_elements = sum(
        row["recoveredElementCount"] for row in families
        if row["cookedPixelShaderStatus"] == "EXTRACTED")
    expected_summary = {
        "authoredElementCount": parent_retained + parent_lost,
        "parentRetainedElementCount": parent_retained,
        "parentLostElementCount": parent_lost,
        "parentLostWithChildPathElementCount": orphan_element_count,
        "parentLostWithoutChildPathElementCount":
            parent_lost_without_child,
        "distinctOrphanChildCount": len(children),
        "resolvedChildCount": len(resolved_rows),
        "resolvedByCounts": dict(sorted(resolved_by_counts.items())),
        "blockedChildCount": len(children) - len(resolved_rows),
        "recoveredElementCount": sum(recovered_parent_counts.values()),
        "recoveredParentMaterialCount": len(families),
        "recoveredParentsAlreadyInDenominator": len(known_parents),
        "recoveredParentsNewToDenominator":
            len(families) - len(known_parents),
        "recoveredElementsWithExtractedDxbc": extracted_elements,
        "blockerCounts": dict(sorted(blocker_counts.items())),
    }
    for key, value in expected_summary.items():
        require(summary.get(key) == value,
                f"child-parent receipt summary.{key} is inconsistent")
    if enforce_canonical_denominators:
        expected_denominators = {
            "authoredElementCount": EXPECTED_AUTHORED_ELEMENT_COUNT,
            "parentRetainedElementCount":
                EXPECTED_PARENT_RETAINED_ELEMENT_COUNT,
            "parentLostElementCount": EXPECTED_PARENT_LOST_ELEMENT_COUNT,
            "parentLostWithChildPathElementCount":
                EXPECTED_PARENT_LOST_WITH_CHILD_PATH_ELEMENT_COUNT,
            "parentLostWithoutChildPathElementCount":
                EXPECTED_PARENT_LOST_WITHOUT_CHILD_PATH_ELEMENT_COUNT,
            "distinctOrphanChildCount":
                EXPECTED_DISTINCT_ORPHAN_CHILD_COUNT,
        }
        for key, value in expected_denominators.items():
            require(summary.get(key) == value,
                    f"canonical child-parent denominator {key} drifted")


def serialize_index(index: dict[str, Any]) -> bytes:
    return (json.dumps(index, indent=2, ensure_ascii=False) + "\n").encode(
        "utf-8")


def parse_staged_index(
        staged: bytes,
        enforce_canonical_denominators: bool) -> dict[str, Any]:
    require(b"\r" not in staged,
            "child-parent receipt must use LF line endings")
    require(staged.endswith(b"\n"),
            "child-parent receipt must end with a newline")
    try:
        document = json.loads(staged.decode("utf-8"))
    except (UnicodeError, json.JSONDecodeError) as error:
        raise ResolutionError(
            f"staged child-parent receipt is not valid UTF-8 JSON: {error}") \
            from error
    validate_resolution_contract(
        document,
        enforce_canonical_denominators=enforce_canonical_denominators)
    return document


def write_index(
        path: Path,
        index: dict[str, Any],
        enforce_canonical_denominators: bool) -> None:
    """Validate, stage, re-parse, then atomically commit one complete receipt."""
    validate_resolution_contract(
        index,
        enforce_canonical_denominators=enforce_canonical_denominators)
    staged = serialize_index(index)
    parsed = parse_staged_index(staged, enforce_canonical_denominators)
    require(parsed == index, "staged child-parent receipt changed on parse")
    path.parent.mkdir(parents=True, exist_ok=True)
    descriptor, temporary_name = tempfile.mkstemp(
        prefix=f".{path.name}.", suffix=".tmp", dir=path.parent)
    temporary = Path(temporary_name)
    try:
        with os.fdopen(descriptor, "wb") as stream:
            stream.write(staged)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary, path)
    except Exception:
        temporary.unlink(missing_ok=True)
        raise


def same_path(left: Path, right: Path) -> bool:
    return left.resolve() == right.resolve()


def protect_canonical_output(
        output: Path,
        authored: Path,
        source_pack_manifest: Path,
        cooked_pixel_shaders: Path,
        named_abi: Path) -> None:
    if not same_path(output, DEFAULT_OUTPUT):
        return
    expected = (
        (authored, AUTHORED_DIRECTORY, "authored directory"),
        (source_pack_manifest, DEFAULT_SOURCE_PACK_MANIFEST,
         "source pack manifest"),
        (cooked_pixel_shaders, COOKED_PIXEL_SHADERS,
         "cooked shader receipt"),
        (named_abi, NAMED_ABI, "named ABI receipt"),
    )
    for actual, canonical, description in expected:
        require(same_path(actual, canonical),
                f"custom {description} cannot overwrite the canonical "
                "child-parent receipt; provide a non-canonical --output")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--authored", type=Path, default=AUTHORED_DIRECTORY)
    parser.add_argument("--source-pack-manifest", type=Path,
                        default=DEFAULT_SOURCE_PACK_MANIFEST)
    parser.add_argument("--cooked-pixel-shaders", type=Path,
                        default=COOKED_PIXEL_SHADERS)
    parser.add_argument("--named-abi", type=Path, default=NAMED_ABI)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--check", action="store_true",
                        help="Fail when the generated contract differs from "
                             "the checked-in file.")
    parser.add_argument("--verbose", action="store_true")
    arguments = parser.parse_args(argv)

    canonical_inputs = all((
        same_path(arguments.authored, AUTHORED_DIRECTORY),
        same_path(arguments.source_pack_manifest,
                  DEFAULT_SOURCE_PACK_MANIFEST),
        same_path(arguments.cooked_pixel_shaders, COOKED_PIXEL_SHADERS),
        same_path(arguments.named_abi, NAMED_ABI),
    ))
    try:
        protect_canonical_output(
            arguments.output, arguments.authored,
            arguments.source_pack_manifest,
            arguments.cooked_pixel_shaders, arguments.named_abi)
        index = build_resolution(
            arguments.authored, arguments.source_pack_manifest,
            arguments.cooked_pixel_shaders, arguments.named_abi,
            canonical_inputs, arguments.verbose)
        if arguments.check:
            require(arguments.output.is_file(), "contract is absent")
            raw = arguments.output.read_bytes()
            current = parse_staged_index(raw, canonical_inputs)
            require(raw == serialize_index(current),
                    "checked-in contract formatting is not canonical")
            require(current == index,
                    "generated contract differs from the checked-in file")
            print("PASS")
            return 0
        write_index(arguments.output, index, canonical_inputs)
    except (OSError, ResolutionError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 2

    summary = index["summary"]
    print(f"WROTE: {arguments.output}")
    print(
        f"RESULT: orphanChildren={summary['distinctOrphanChildCount']} "
        f"resolved={summary['resolvedChildCount']} "
        f"recoveredElements={summary['recoveredElementCount']} "
        f"parents={summary['recoveredParentMaterialCount']} "
        f"newFamilies={summary['recoveredParentsNewToDenominator']} "
        f"withExtractedDxbc={summary['recoveredElementsWithExtractedDxbc']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
