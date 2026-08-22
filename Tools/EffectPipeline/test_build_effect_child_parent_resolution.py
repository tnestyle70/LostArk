#!/usr/bin/env python3
"""Unit tests for the orphan child -> parent material recovery.

The value of this contract is that a recovered parent is the object the
source pack actually declares, not a guess that happens to close a row.  These
tests pin the three ways guessing could creep in and the receipt boundary
around that recovery: authenticated upstream artifacts, exact denominators,
duplicate rejection, LF-only staging and atomic commit.
"""

from __future__ import annotations

import copy
import hashlib
import json
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))

from build_effect_child_parent_resolution import (  # noqa: E402
    BLOCKER_CHAIN_UNTERMINATED,
    BLOCKER_EXPORT_AMBIGUOUS,
    BLOCKER_LEAF_ABSENT_IN_PACK,
    BLOCKER_LEAF_SEARCH_AMBIGUOUS,
    BLOCKER_PARENT_PROPERTY_ABSENT,
    COOKED_PIXEL_SHADERS,
    COOKED_PIXEL_SHADERS_SCHEMA,
    DEFAULT_OUTPUT,
    DEFAULT_SOURCE_PACK_MANIFEST,
    INSTANCE_MATERIAL_CLASS,
    NAMED_ABI,
    NAMED_ABI_SCHEMA,
    PENDING,
    RESOLVED_DECLARED_PACKAGE,
    RESOLVED_LEAF_SEARCH,
    ROOT_MATERIAL_CLASS,
    STATUS_BLOCKED,
    STATUS_NAMED_MAPPING_RESOLVED,
    STATUS_RESOLVED,
    LeafIndex,
    ResolutionError,
    canonical_sha256,
    canonical_object_path,
    collect_orphan_children,
    load_validated_package_index,
    parse_staged_index,
    protect_canonical_output,
    read_artifact,
    resolve_child,
    select_export,
    serialize_index,
    validate_cooked_contract,
    validate_named_abi_contract,
    validate_resolution_contract,
    write_index,
)


def sign_artifact(document):
    result = copy.deepcopy(document)
    result.pop("artifactSha256", None)
    result["artifactSha256"] = canonical_sha256(result)
    return result


def cooked_fixture():
    return sign_artifact({
        "schema": COOKED_PIXEL_SHADERS_SCHEMA,
        "formatVersion": 1,
        "summary": {
            "familyCount": 2,
            "extractedCount": 1,
            "blockedCount": 1,
            "blockerCounts": {"no program": 1},
        },
        "families": [
            {
                "parentMaterialPath": "pkg.m.root",
                "status": "EXTRACTED",
                "dxbcSha256": "1" * 64,
            },
            {
                "parentMaterialPath": "pkg.m.blocked",
                "status": "BLOCKED",
                "blocker": "no program",
            },
        ],
    })


def named_fixture(cooked, cooked_identity):
    return sign_artifact({
        "schema": NAMED_ABI_SCHEMA,
        "formatVersion": 1,
        "inputs": {
            "cookedPixelShadersArtifactSha256":
                cooked["artifactSha256"],
            "cookedPixelShadersRawSha256":
                cooked_identity["rawSha256"],
            "cookedPixelShadersByteSize": cooked_identity["byteSize"],
        },
        "summary": {
            "familyCount": 1,
            "resolvedNamedMappingCount": 1,
            "blockedCount": 0,
            "blockerCounts": {},
        },
        "families": [{
            "parentMaterialPath": "pkg.m.root",
            "dxbcSha256": "1" * 64,
            "status": STATUS_NAMED_MAPPING_RESOLVED,
        }],
    })


def blocked_named_fixture(cooked, cooked_identity):
    return sign_artifact({
        "schema": NAMED_ABI_SCHEMA,
        "formatVersion": 1,
        "inputs": {
            "cookedPixelShadersArtifactSha256":
                cooked["artifactSha256"],
            "cookedPixelShadersRawSha256":
                cooked_identity["rawSha256"],
            "cookedPixelShadersByteSize": cooked_identity["byteSize"],
        },
        "summary": {
            "familyCount": 1,
            "resolvedNamedMappingCount": 0,
            "blockedCount": 1,
            "blockerCounts": {
                "NATIVE_BINDING_ARRAY_CANDIDATE_AMBIGUOUS": 1,
            },
        },
        "families": [{
            "parentMaterialPath": "pkg.m.root",
            "status": STATUS_BLOCKED,
            "blocker": {
                "reasonCode": "NATIVE_BINDING_ARRAY_CANDIDATE_AMBIGUOUS",
                "candidateCount": 2,
            },
        }],
    })


def resolution_fixture():
    child = {
        "childMaterialPath": "pkg.mi.child",
        "elementCount": 2,
        "effectAssetIdCount": 1,
        "status": STATUS_RESOLVED,
        "resolvedBy": RESOLVED_DECLARED_PACKAGE,
        "parentChain": ["pkg.m.root"],
        "parentMaterialPath": "pkg.m.root",
        "canonicalParentMaterialPath": "pkg.m.root",
        "parentDeclaringPackage": "pkg",
        "knownFamilyPath": "pkg.m.root",
        "cookedPixelShaderStatus": "EXTRACTED",
        "namedAbiClosed": True,
        "familyAlreadyInDenominator": True,
    }
    child["rowSha256"] = canonical_sha256(child)
    family = {
        "canonicalParentMaterialPath": "pkg.m.root",
        "knownFamilyPath": "pkg.m.root",
        "recoveredElementCount": 2,
        "cookedPixelShaderStatus": "EXTRACTED",
        "namedAbiClosed": True,
        "alreadyInDenominator": True,
    }
    family["rowSha256"] = canonical_sha256(family)
    return sign_artifact({
        "schema": "lostark.effect-child-parent-resolution",
        "formatVersion": 1,
        "identity": {
            "scope": "AUTHORED_CORPUS_ORPHAN_CHILD_MATERIALS",
            "admits": "PARENT_MATERIAL_JOIN_KEY_ONLY",
        },
        "inputs": {
            "authoredDirectory": "Data/Effects/Authored",
            "authoredDocumentCount": 1,
            "authoredDocumentSetSha256": "2" * 64,
            "sourcePackManifest": "source_pack_manifest.json",
            "sourcePackManifestRawSha256": "3" * 64,
            "sourcePackManifestByteSize": 100,
            "sourcePackageCount": 1,
            "sourcePackageByteSize": 10,
            "sourcePackageSetSha256": "4" * 64,
            "cookedPixelShaders": "cooked.json",
            "cookedPixelShadersArtifactSha256": "5" * 64,
            "cookedPixelShadersRawSha256": "6" * 64,
            "cookedPixelShadersByteSize": 20,
            "namedAbi": "named.json",
            "namedAbiArtifactSha256": "7" * 64,
            "namedAbiRawSha256": "8" * 64,
            "namedAbiByteSize": 30,
            "packagesOpened": 1,
            "leafSearchSweeps": 0,
        },
        "summary": {
            "authoredElementCount": 4,
            "parentRetainedElementCount": 1,
            "parentLostElementCount": 3,
            "parentLostWithChildPathElementCount": 2,
            "parentLostWithoutChildPathElementCount": 1,
            "distinctOrphanChildCount": 1,
            "resolvedChildCount": 1,
            "resolvedByCounts": {RESOLVED_DECLARED_PACKAGE: 1},
            "blockedChildCount": 0,
            "recoveredElementCount": 2,
            "recoveredParentMaterialCount": 1,
            "recoveredParentsAlreadyInDenominator": 1,
            "recoveredParentsNewToDenominator": 0,
            "recoveredElementsWithExtractedDxbc": 2,
            "blockerCounts": {},
        },
        "families": [family],
        "children": [child],
    })


class FakePackageCache:
    """Stands in for the staged pack.

    `rows` mirrors `export_material_rows`: leaf -> Material/MIC export rows.
    `parents` supplies the `Parent` reference a MIC would serialize.
    """

    def __init__(self, packages, parents):
        self._packages = packages
        self._parents = parents

    def has(self, package_name):
        return package_name in self._packages

    def rows(self, package_name):
        return self._packages[package_name]

    def package(self, package_name):
        return package_name

    def parent_of(self, leaf):
        return self._parents.get(leaf)


def install_parent_reader(cache):
    """Point the module's tagged-property read at the fake cache."""
    import build_effect_child_parent_resolution as module

    original = module.parent_reference
    module.parent_reference = lambda package, row: cache.parent_of(
        row["leaf"])
    return original


def material_row(leaf, object_path):
    return {"className": ROOT_MATERIAL_CLASS, "objectPath": object_path,
            "leaf": leaf, "serialOffset": 0, "serialByteSize": 0}


def instance_row(leaf, object_path):
    return {"className": INSTANCE_MATERIAL_CLASS, "objectPath": object_path,
            "leaf": leaf, "serialOffset": 0, "serialByteSize": 0}


class CanonicalObjectPathTests(unittest.TestCase):

    def test_group_only_path_is_qualified_with_its_package(self):
        self.assertEqual(
            canonical_object_path("fx_m_mi_00", "fx_m.fx_c_pa_lensflare_01_ad"),
            "fx_m_mi_00.fx_m.fx_c_pa_lensflare_01_ad")

    def test_already_qualified_path_is_left_alone(self):
        self.assertEqual(
            canonical_object_path(
                "fx_mastermaterial",
                "fx_mastermaterial.fx_mm.fx_mm_simple_01_ad"),
            "fx_mastermaterial.fx_mm.fx_mm_simple_01_ad")

    def test_qualification_is_case_insensitive(self):
        self.assertEqual(
            canonical_object_path("FX_M_MI_00", "fx_m_mi_00.fx_m.leaf"),
            "fx_m_mi_00.fx_m.leaf")


class SelectExportTests(unittest.TestCase):

    def test_single_export_is_selected(self):
        rows = {"leaf": [material_row("leaf", "grp.leaf")]}
        row, blocker = select_export(rows, "LEAF")
        self.assertIsNone(blocker)
        self.assertEqual(row["objectPath"], "grp.leaf")

    def test_absent_leaf_reports_a_blocker(self):
        row, blocker = select_export({}, "leaf")
        self.assertIsNone(row)
        self.assertIsNotNone(blocker)

    def test_two_exports_of_one_leaf_are_ambiguous(self):
        rows = {"leaf": [material_row("leaf", "a.leaf"),
                         material_row("leaf", "b.leaf")]}
        row, blocker = select_export(rows, "leaf")
        self.assertIsNone(row)
        self.assertEqual(blocker, BLOCKER_EXPORT_AMBIGUOUS)


class LeafIndexReduceTests(unittest.TestCase):

    def test_absent_leaf_is_blocked(self):
        self.assertEqual(
            LeafIndex._reduce([])["blocker"], BLOCKER_LEAF_ABSENT_IN_PACK)

    def test_mixed_classes_are_ambiguous(self):
        entry = LeafIndex._reduce([
            {"className": ROOT_MATERIAL_CLASS, "objectPath": "a.leaf",
             "packageName": "p"},
            {"className": INSTANCE_MATERIAL_CLASS, "parentPath": "a.parent",
             "packageName": "q"},
        ])
        self.assertEqual(entry["blocker"], BLOCKER_LEAF_SEARCH_AMBIGUOUS)

    def test_materials_naming_different_objects_are_ambiguous(self):
        entry = LeafIndex._reduce([
            {"className": ROOT_MATERIAL_CLASS, "objectPath": "a.leaf",
             "packageName": "p"},
            {"className": ROOT_MATERIAL_CLASS, "objectPath": "b.leaf",
             "packageName": "q"},
        ])
        self.assertEqual(entry["blocker"], BLOCKER_LEAF_SEARCH_AMBIGUOUS)

    def test_materials_naming_one_object_are_accepted(self):
        entry = LeafIndex._reduce([
            {"className": ROOT_MATERIAL_CLASS, "objectPath": "a.leaf",
             "packageName": "p"},
            {"className": ROOT_MATERIAL_CLASS, "objectPath": "a.leaf",
             "packageName": "p"},
        ])
        self.assertEqual(entry["objectPath"], "a.leaf")
        self.assertEqual(entry["candidateCount"], 2)

    def test_same_relative_material_path_in_two_packages_is_ambiguous(self):
        entry = LeafIndex._reduce([
            {"className": ROOT_MATERIAL_CLASS, "objectPath": "a.leaf",
             "packageName": "p"},
            {"className": ROOT_MATERIAL_CLASS, "objectPath": "a.leaf",
             "packageName": "q"},
        ])
        self.assertEqual(entry["blocker"], BLOCKER_LEAF_SEARCH_AMBIGUOUS)

    def test_instances_disagreeing_on_parent_are_ambiguous(self):
        entry = LeafIndex._reduce([
            {"className": INSTANCE_MATERIAL_CLASS, "parentPath": "a.p",
             "packageName": "p"},
            {"className": INSTANCE_MATERIAL_CLASS, "parentPath": "b.p",
             "packageName": "q"},
        ])
        self.assertEqual(entry["blocker"], BLOCKER_LEAF_SEARCH_AMBIGUOUS)

    def test_instance_without_a_parent_is_blocked(self):
        entry = LeafIndex._reduce([
            {"className": INSTANCE_MATERIAL_CLASS, "parentPath": None,
             "packageName": "p"},
        ])
        self.assertEqual(entry["blocker"], BLOCKER_PARENT_PROPERTY_ABSENT)


class LeafIndexSweepTests(unittest.TestCase):

    def make_index(self):
        index = LeafIndex({"fx_broken": Path("Pack/FX_BROKEN.upk")})
        index.request("target_leaf")
        return index

    def test_package_load_failure_aborts_complete_leaf_search(self):
        index = self.make_index()
        with mock.patch(
                "build_effect_child_parent_resolution.load_package",
                side_effect=RuntimeError("encrypted payload is truncated")):
            with self.assertRaisesRegex(
                    ResolutionError,
                    "leaf search package load failed: "
                    ".*package=fx_broken.*FX_BROKEN[.]upk.*"
                    "RuntimeError: encrypted payload is truncated"):
                index.sweep(False)

    def test_material_export_scan_failure_aborts_complete_leaf_search(self):
        index = self.make_index()
        with mock.patch(
                "build_effect_child_parent_resolution.load_package",
                return_value=object()), mock.patch(
                    "build_effect_child_parent_resolution."
                    "export_material_rows",
                    side_effect=ValueError("export table is malformed")):
            with self.assertRaisesRegex(
                    ResolutionError,
                    "leaf search material export scan failed: "
                    ".*package=fx_broken.*FX_BROKEN[.]upk.*"
                    "ValueError: export table is malformed"):
                index.sweep(False)


class ResolveChildTests(unittest.TestCase):

    def setUp(self):
        import build_effect_child_parent_resolution as module
        self.module = module
        self.original_parent_reference = module.parent_reference

    def tearDown(self):
        self.module.parent_reference = self.original_parent_reference

    def test_declared_package_chain_reaches_the_root_material(self):
        cache = FakePackageCache(
            packages={
                "pkg_mi": {"child": [instance_row("child", "mi.child")]},
                "pkg_m": {"root": [material_row("root", "m.root")]},
            },
            parents={"child": "pkg_m.m.root"})
        install_parent_reader(cache)
        result = resolve_child("pkg_mi.mi.child", cache, LeafIndex({}))
        self.assertEqual(result["status"], STATUS_RESOLVED)
        self.assertEqual(result["resolvedBy"], RESOLVED_DECLARED_PACKAGE)
        self.assertEqual(result["parentMaterialPath"], "pkg_m.m.root")
        self.assertEqual(result["canonicalParentMaterialPath"], "pkg_m.m.root")
        self.assertEqual(result["parentChain"], ["pkg_m.m.root"])

    def test_group_only_parent_is_pending_until_a_sweep_runs(self):
        cache = FakePackageCache(
            packages={"pkg_mi": {"child": [instance_row("child", "mi.child")]}},
            parents={"child": "m.root"})
        install_parent_reader(cache)
        leaf_index = LeafIndex({})
        result = resolve_child("pkg_mi.mi.child", cache, leaf_index)
        self.assertEqual(result["status"], PENDING)
        self.assertTrue(leaf_index.has_pending())

    def test_leaf_search_result_is_requalified_with_its_package(self):
        cache = FakePackageCache(
            packages={"pkg_mi": {"child": [instance_row("child", "mi.child")]}},
            parents={"child": "m.root"})
        install_parent_reader(cache)
        leaf_index = LeafIndex({})
        leaf_index.request("root")
        leaf_index._entries["root"] = {
            "className": ROOT_MATERIAL_CLASS,
            "objectPath": "m.root",
            "packageName": "pkg_m",
            "candidateCount": 1,
        }
        result = resolve_child("pkg_mi.mi.child", cache, leaf_index)
        self.assertEqual(result["status"], STATUS_RESOLVED)
        self.assertEqual(result["resolvedBy"], RESOLVED_LEAF_SEARCH)
        self.assertEqual(result["parentMaterialPath"], "m.root")
        self.assertEqual(
            result["canonicalParentMaterialPath"], "pkg_m.m.root")

    def test_self_referencing_parent_fails_closed(self):
        cache = FakePackageCache(
            packages={"pkg": {"child": [instance_row("child", "mi.child")]}},
            parents={"child": "pkg.mi.child"})
        install_parent_reader(cache)
        result = resolve_child("pkg.mi.child", cache, LeafIndex({}))
        self.assertEqual(result["status"], STATUS_BLOCKED)
        self.assertEqual(result["blocker"], BLOCKER_CHAIN_UNTERMINATED)

    def test_missing_parent_property_is_blocked(self):
        cache = FakePackageCache(
            packages={"pkg": {"child": [instance_row("child", "mi.child")]}},
            parents={})
        install_parent_reader(cache)
        result = resolve_child("pkg.mi.child", cache, LeafIndex({}))
        self.assertEqual(result["status"], STATUS_BLOCKED)
        self.assertEqual(result["blocker"], BLOCKER_PARENT_PROPERTY_ABSENT)

    def test_ambiguous_declared_export_fails_without_leaf_fallback(self):
        cache = FakePackageCache(
            packages={"pkg": {
                "child": [instance_row("child", "mi.a.child"),
                          instance_row("child", "mi.b.child")],
            }},
            parents={})
        result = resolve_child("pkg.mi.child", cache, LeafIndex({}))
        self.assertEqual(result["status"], STATUS_BLOCKED)
        self.assertEqual(result["blocker"], BLOCKER_EXPORT_AMBIGUOUS)


class CollectOrphanChildrenTests(unittest.TestCase):

    def test_only_elements_without_a_parent_are_collected(self):
        import json
        import tempfile

        document = {
            "effectAssetId": "effect.sample",
            "elements": [
                {"id": "retained", "material": {
                              "sourceMaterialPath": "pkg.mi.a",
                              "sourceProfile": {
                                  "parentMaterialPath": "pkg.m.root"}}},
                {"id": "orphan", "material": {
                              "sourceMaterialPath": "pkg.mi.b",
                              "sourceProfile": {"enabled": False}}},
                {"id": "no-child", "material": {"sourceProfile": {}}},
            ],
        }
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "effect.sample.effect.json"
            path.write_text(json.dumps(document), encoding="utf-8")
            orphans, census, identity = collect_orphan_children(
                Path(directory))
        self.assertEqual(set(orphans), {"pkg.mi.b"})
        self.assertEqual(identity["documentCount"], 1)
        self.assertEqual(census["elements"], 3)
        self.assertEqual(census["parentRetained"], 1)
        self.assertEqual(census["parentLost"], 2)
        self.assertEqual(census["parentLostWithChildPath"], 1)
        self.assertEqual(census["parentLostWithoutChildPath"], 1)

    def test_duplicate_effect_asset_id_is_rejected(self):
        document = {"effectAssetId": "effect.duplicate", "elements": []}
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name in ("one.effect.json", "two.effect.json"):
                (root / name).write_text(json.dumps(document), encoding="utf-8")
            with self.assertRaisesRegex(ResolutionError, "duplicates effectAssetId"):
                collect_orphan_children(root)

    def test_duplicate_element_id_is_rejected(self):
        document = {
            "effectAssetId": "effect.duplicate.element",
            "elements": [{"id": "same"}, {"id": "same"}],
        }
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "effect.effect.json").write_text(
                json.dumps(document), encoding="utf-8")
            with self.assertRaisesRegex(ResolutionError, "duplicates element id"):
                collect_orphan_children(root)


class UpstreamArtifactTests(unittest.TestCase):

    def test_read_artifact_rejects_schema_and_self_hash_drift(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "artifact.json"
            document = cooked_fixture()
            path.write_text(json.dumps(document), encoding="utf-8")
            parsed, _ = read_artifact(
                path, COOKED_PIXEL_SHADERS_SCHEMA, "fixture")
            self.assertEqual(parsed["artifactSha256"],
                             document["artifactSha256"])
            wrong_schema = sign_artifact({**document, "schema": "wrong"})
            path.write_text(json.dumps(wrong_schema), encoding="utf-8")
            with self.assertRaisesRegex(ResolutionError, "schema"):
                read_artifact(path, COOKED_PIXEL_SHADERS_SCHEMA, "fixture")
            drifted = cooked_fixture()
            drifted["summary"]["familyCount"] = 99
            path.write_text(json.dumps(drifted), encoding="utf-8")
            with self.assertRaisesRegex(ResolutionError, "artifactSha256 drifted"):
                read_artifact(path, COOKED_PIXEL_SHADERS_SCHEMA, "fixture")

    def test_cooked_duplicate_family_is_rejected(self):
        document = cooked_fixture()
        document["families"].append(copy.deepcopy(document["families"][0]))
        document["summary"]["familyCount"] += 1
        document["summary"]["extractedCount"] += 1
        document = sign_artifact(document)
        with self.assertRaisesRegex(ResolutionError, "duplicates family"):
            validate_cooked_contract(document, False)

    def test_canonical_cooked_denominator_is_enforced(self):
        with self.assertRaisesRegex(ResolutionError, "denominator"):
            validate_cooked_contract(cooked_fixture(), True)

    def test_named_abi_must_cover_exact_extracted_family_set(self):
        cooked = cooked_fixture()
        identity = {"rawSha256": "a" * 64, "byteSize": 10}
        named = named_fixture(cooked, identity)
        named["families"] = []
        named["summary"] = {
            "familyCount": 0,
            "resolvedNamedMappingCount": 0,
            "blockedCount": 0,
            "blockerCounts": {},
        }
        named = sign_artifact(named)
        cooked_rows = validate_cooked_contract(cooked, False)
        with self.assertRaisesRegex(ResolutionError, "family set"):
            validate_named_abi_contract(
                named, cooked_rows, cooked, identity, False)

    def test_named_abi_duplicate_family_is_rejected(self):
        cooked = cooked_fixture()
        identity = {"rawSha256": "a" * 64, "byteSize": 10}
        named = named_fixture(cooked, identity)
        named["families"].append(copy.deepcopy(named["families"][0]))
        named["summary"]["familyCount"] = 2
        named["summary"]["resolvedNamedMappingCount"] = 2
        named = sign_artifact(named)
        cooked_rows = validate_cooked_contract(cooked, False)
        with self.assertRaisesRegex(ResolutionError, "duplicates family"):
            validate_named_abi_contract(
                named, cooked_rows, cooked, identity, False)

    def test_named_abi_must_pin_exact_cooked_bytes(self):
        cooked = cooked_fixture()
        identity = {"rawSha256": "a" * 64, "byteSize": 10}
        named = named_fixture(cooked, identity)
        named["inputs"]["cookedPixelShadersRawSha256"] = "b" * 64
        named = sign_artifact(named)
        cooked_rows = validate_cooked_contract(cooked, False)
        with self.assertRaisesRegex(ResolutionError, "different cooked bytes"):
            validate_named_abi_contract(
                named, cooked_rows, cooked, identity, False)

    def test_named_abi_structured_blocker_is_counted_by_reason_code(self):
        cooked = cooked_fixture()
        identity = {"rawSha256": "a" * 64, "byteSize": 10}
        named = blocked_named_fixture(cooked, identity)
        cooked_rows = validate_cooked_contract(cooked, False)
        rows = validate_named_abi_contract(
            named, cooked_rows, cooked, identity, False)
        self.assertEqual(
            rows["pkg.m.root"]["blocker"],
            {
                "reasonCode": "NATIVE_BINDING_ARRAY_CANDIDATE_AMBIGUOUS",
                "candidateCount": 2,
            },
        )

    def test_named_abi_rejects_malformed_structured_blocker(self):
        cooked = cooked_fixture()
        identity = {"rawSha256": "a" * 64, "byteSize": 10}
        cooked_rows = validate_cooked_contract(cooked, False)
        malformed = (
            ("legacy string", "legacy blocker", "blocker must be an object"),
            ({"reasonCode": "REASON"}, "missing candidateCount",
             "blocker fields are malformed"),
            ({"reasonCode": "REASON", "candidateCount": 2,
              "detail": "unexpected"}, "unexpected blocker field",
             "blocker fields are malformed"),
        )
        for blocker, label, message in malformed:
            with self.subTest(label=label):
                named = blocked_named_fixture(cooked, identity)
                named["families"][0]["blocker"] = blocker
                named = sign_artifact(named)
                with self.assertRaisesRegex(ResolutionError, message):
                    validate_named_abi_contract(
                        named, cooked_rows, cooked, identity, False)

    def test_named_abi_rejects_malformed_blocker_reason_code(self):
        cooked = cooked_fixture()
        identity = {"rawSha256": "a" * 64, "byteSize": 10}
        cooked_rows = validate_cooked_contract(cooked, False)
        for reason_code in (None, "", "   ", 7):
            with self.subTest(reasonCode=reason_code):
                named = blocked_named_fixture(cooked, identity)
                named["families"][0]["blocker"]["reasonCode"] = reason_code
                named = sign_artifact(named)
                with self.assertRaisesRegex(ResolutionError,
                                             "reasonCode is malformed"):
                    validate_named_abi_contract(
                        named, cooked_rows, cooked, identity, False)

    def test_named_abi_rejects_malformed_blocker_candidate_count(self):
        cooked = cooked_fixture()
        identity = {"rawSha256": "a" * 64, "byteSize": 10}
        cooked_rows = validate_cooked_contract(cooked, False)
        for candidate_count in (None, True, 1, 0, -1, 2.0, "2"):
            with self.subTest(candidateCount=candidate_count):
                named = blocked_named_fixture(cooked, identity)
                named["families"][0]["blocker"]["candidateCount"] = (
                    candidate_count)
                named = sign_artifact(named)
                with self.assertRaisesRegex(
                        ResolutionError,
                        "candidateCount must be an integer greater than one"):
                    validate_named_abi_contract(
                        named, cooked_rows, cooked, identity, False)


class SourcePackManifestTests(unittest.TestCase):

    def make_manifest(self, root):
        package = root / "Pack" / "sample.upk"
        package.parent.mkdir()
        package.write_bytes(b"source-package")
        digest = hashlib.sha256(package.read_bytes()).hexdigest()
        manifest = {
            "schemaVersion": 1,
            "summary": {"packageCount": 1,
                        "byteSize": package.stat().st_size},
            "packages": [{
                "logicalPackage": "FX_SAMPLE",
                "resolved": True,
                "relativePath": "Pack/sample.upk",
                "sha256": digest,
                "byteSize": package.stat().st_size,
            }],
        }
        path = root / "source_pack_manifest.json"
        path.write_text(json.dumps(manifest), encoding="utf-8")
        return path, manifest

    def test_manifest_and_package_bytes_are_authenticated(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            path, _ = self.make_manifest(root)
            index, identity = load_validated_package_index(path, False)
            self.assertEqual(set(index), {"fx_sample"})
            self.assertEqual(identity["packageCount"], 1)
            index["fx_sample"].write_bytes(b"drift")
            with self.assertRaisesRegex(ResolutionError, "SHA-256 drifted"):
                load_validated_package_index(path, False)

    def test_duplicate_logical_package_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            path, manifest = self.make_manifest(root)
            duplicate = copy.deepcopy(manifest["packages"][0])
            duplicate["relativePath"] = "Pack/duplicate.upk"
            (root / duplicate["relativePath"]).write_bytes(b"source-package")
            manifest["packages"].append(duplicate)
            manifest["summary"]["packageCount"] = 2
            manifest["summary"]["byteSize"] *= 2
            path.write_text(json.dumps(manifest), encoding="utf-8")
            with self.assertRaisesRegex(ResolutionError,
                                        "duplicates logical package"):
                load_validated_package_index(path, False)


class ResolutionReceiptTests(unittest.TestCase):

    def test_valid_receipt_round_trips_with_lf(self):
        receipt = resolution_fixture()
        validate_resolution_contract(receipt, False)
        staged = serialize_index(receipt)
        self.assertNotIn(b"\r", staged)
        self.assertEqual(parse_staged_index(staged, False), receipt)

    def test_crlf_receipt_is_rejected(self):
        staged = serialize_index(resolution_fixture()).replace(b"\n", b"\r\n")
        with self.assertRaisesRegex(ResolutionError, "LF line endings"):
            parse_staged_index(staged, False)

    def test_duplicate_child_is_rejected_even_with_valid_artifact_hash(self):
        receipt = resolution_fixture()
        receipt["children"].append(copy.deepcopy(receipt["children"][0]))
        receipt["summary"]["distinctOrphanChildCount"] = 2
        receipt = sign_artifact(receipt)
        with self.assertRaisesRegex(ResolutionError, "duplicates child"):
            validate_resolution_contract(receipt, False)

    def test_row_hash_drift_is_rejected(self):
        receipt = resolution_fixture()
        receipt["children"][0]["elementCount"] = 3
        receipt = sign_artifact(receipt)
        with self.assertRaisesRegex(ResolutionError, "rowSha256 drifted"):
            validate_resolution_contract(receipt, False)

    def test_invalid_stage_does_not_overwrite_existing_receipt(self):
        receipt = resolution_fixture()
        receipt["families"].append(copy.deepcopy(receipt["families"][0]))
        receipt = sign_artifact(receipt)
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "receipt.json"
            path.write_bytes(b"preserve-me")
            with self.assertRaisesRegex(ResolutionError, "duplicates family"):
                write_index(path, receipt, False)
            self.assertEqual(path.read_bytes(), b"preserve-me")

    def test_failed_atomic_replace_preserves_existing_receipt(self):
        receipt = resolution_fixture()
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            path = root / "receipt.json"
            path.write_bytes(b"preserve-me")
            with mock.patch(
                    "build_effect_child_parent_resolution.os.replace",
                    side_effect=OSError("replace failed")):
                with self.assertRaisesRegex(OSError, "replace failed"):
                    write_index(path, receipt, False)
            self.assertEqual(path.read_bytes(), b"preserve-me")
            self.assertEqual(list(root.glob(".receipt.json.*.tmp")), [])

    def test_custom_input_cannot_overwrite_canonical_output(self):
        with tempfile.TemporaryDirectory() as directory:
            with self.assertRaisesRegex(ResolutionError,
                                        "cannot overwrite the canonical"):
                protect_canonical_output(
                    DEFAULT_OUTPUT, Path(directory),
                    DEFAULT_SOURCE_PACK_MANIFEST,
                    COOKED_PIXEL_SHADERS, NAMED_ABI)


if __name__ == "__main__":
    unittest.main()
