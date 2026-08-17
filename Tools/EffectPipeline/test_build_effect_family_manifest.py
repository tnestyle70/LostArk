#!/usr/bin/env python3
"""Regression tests for the Effect parent-material family manifest."""

from __future__ import annotations

import importlib.util
import json
import tempfile
import unittest
from pathlib import Path

SCRIPT_PATH = Path(__file__).resolve().with_name(
    "build_effect_family_manifest.py")
SPEC = importlib.util.spec_from_file_location(
    "build_effect_family_manifest", SCRIPT_PATH)
assert SPEC is not None and SPEC.loader is not None
builder = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(builder)


PROPS_SAMPLE = """TwoSided = true
bDisableDepthTest = false
bIsMasked = false
ReferencedTextures[2] = { Texture2D'a' Texture2D'b' }
Expressions[73] =
{
    Expressions[39] = MaterialExpressionScalarParameter'x.materialexpressionscalarparameter_0'
    Expressions[67] = MaterialExpressionStaticSwitchParameter'x.materialexpressionstaticswitchparameter_3'
    Expressions[68] = MaterialExpressionStaticSwitchParameter'x.materialexpressionstaticswitchparameter_5'
}
BlendMode = BLEND_Translucent (2)
CollectedTextureParameters[1] =
{
    CollectedTextureParameters[0] =
    {
        Texture = Texture2D'fx_j_voronoi_tile_01'
        Name = slice_flow_texture
        Group = flow
    }
}
CollectedScalarParameters[2] =
{
    CollectedScalarParameters[0] = { Value=-3.14, Name=slice_rot, Group=None }
    CollectedScalarParameters[1] = { Value=2, Name=opacity_radius, Group=None }
}
CollectedVectorParameters[0] = {}
"""
# The sample above is copied from the real export on purpose: texture entries
# span lines with spaces around '=', scalar entries are one line without them.
# A parser written against either layout alone silently drops the other.


class PropsParsingTests(unittest.TestCase):
    def setUp(self) -> None:
        self.parsed = builder.parse_props(PROPS_SAMPLE)

    def test_header_facts_are_typed_not_strings(self) -> None:
        self.assertEqual(self.parsed["blendMode"], "BLEND_Translucent (2)")
        self.assertIs(self.parsed["twoSided"], True)
        self.assertIs(self.parsed["isMasked"], False)
        self.assertIs(self.parsed["disableDepthTest"], False)

    def test_expression_slots_are_the_declared_count_not_the_survivors(self):
        # 73 declared, 3 serialized: the gap is the cooked topology loss and
        # the manifest must report the declared slot count.
        self.assertEqual(self.parsed["expressionSlots"], 73)

    def test_static_switch_count_drives_permutation_estimates(self) -> None:
        self.assertEqual(self.parsed["staticSwitchCount"], 2)

    def test_named_parameters_keep_name_group_and_value(self) -> None:
        self.assertEqual(
            self.parsed["textureParameters"],
            [{"name": "slice_flow_texture", "group": "flow",
              "texture": "fx_j_voronoi_tile_01"}])
        self.assertEqual(
            self.parsed["scalarParameters"],
            [{"name": "slice_rot", "group": "None", "value": -3.14},
             {"name": "opacity_radius", "group": "None", "value": 2.0}])
        self.assertEqual(self.parsed["vectorParameters"], [])

    def test_duplicate_scalar_names_are_preserved(self) -> None:
        text = PROPS_SAMPLE.replace(
            "CollectedScalarParameters[1] = { Value=2, Name=opacity_radius, Group=None }",
            "CollectedScalarParameters[1] = { Value=2, Name=slice_rot, Group=alpha }")
        names = [row["name"]
                 for row in builder.parse_props(text)["scalarParameters"]]
        self.assertEqual(names, ["slice_rot", "slice_rot"])


class AdmissionTests(unittest.TestCase):
    @staticmethod
    def family(**overrides):
        base = {
            "evidence": {"status": "PRESENT"},
            "carriers": {"sprite": 4},
            "roleSets": [{"roles": ["base"], "occurrenceCount": 4}],
            "renderProfiles": {"additive_one_sided_depth_read": 4},
        }
        base.update(overrides)
        return base

    def test_single_variant_with_evidence_is_ready(self) -> None:
        result = builder.classify_admission(self.family())
        self.assertEqual(
            result["status"], "READY_FOR_SINGLE_FAMILY_IMPLEMENTATION")
        self.assertEqual(result["blockers"], [])

    def test_mixed_carrier_blocks_single_implementation(self) -> None:
        result = builder.classify_admission(
            self.family(carriers={"sprite": 3, "mesh": 1}))
        self.assertEqual(result["status"], "VARIANT_SPLIT_REQUIRED")
        self.assertIn("CARRIER_SPLIT_SPRITE_AND_MESH", result["blockers"])

    def test_role_set_variance_blocks_single_implementation(self) -> None:
        result = builder.classify_admission(self.family(roleSets=[
            {"roles": ["base"], "occurrenceCount": 3},
            {"roles": ["base", "mask"], "occurrenceCount": 1}]))
        self.assertIn("NAMED_ROLE_SET_VARIANCE", result["blockers"])

    def test_render_profile_variance_blocks_single_implementation(self) -> None:
        result = builder.classify_admission(self.family(renderProfiles={
            "additive_one_sided_depth_read": 3,
            "additive_two_sided_depth_read": 1}))
        self.assertIn("RENDER_PROFILE_VARIANCE", result["blockers"])

    def test_missing_evidence_is_reported_separately_from_variance(self) -> None:
        only_evidence = builder.classify_admission(
            self.family(evidence={"status": "ABSENT"}))
        self.assertEqual(only_evidence["status"], "EVIDENCE_REQUIRED")

        both = builder.classify_admission(self.family(
            evidence={"status": "ABSENT"},
            carriers={"sprite": 3, "mesh": 1}))
        self.assertEqual(
            both["status"], "EVIDENCE_REQUIRED_AND_VARIANT_SPLIT_REQUIRED")


class ManifestTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.manifest = builder.build_manifest(
            builder.REPOSITORY_ROOT, builder.DEFAULT_EVIDENCE_ROOT)

    def test_manifest_is_sealed_and_reproducible(self) -> None:
        sealed = dict(self.manifest)
        recorded = sealed.pop("artifactSha256")
        self.assertEqual(builder.canonical_sha256(sealed), recorded)

        rebuilt = builder.build_manifest(
            builder.REPOSITORY_ROOT, builder.DEFAULT_EVIDENCE_ROOT)
        self.assertEqual(rebuilt["artifactSha256"], recorded)

    def test_every_family_row_is_sealed(self) -> None:
        for family in self.manifest["families"]:
            row = dict(family)
            recorded = row.pop("rowSha256")
            self.assertEqual(
                builder.canonical_sha256(row), recorded,
                f"row seal mismatch: {family['parentMaterialPath']}")

    def test_families_are_ranked_by_occurrence(self) -> None:
        counts = [f["occurrenceCount"] for f in self.manifest["families"]]
        self.assertEqual(counts, sorted(counts, reverse=True))

    def test_grouped_totals_agree_with_the_family_rows(self) -> None:
        total = sum(f["groupedOccurrenceCount"]
                    for f in self.manifest["families"])
        self.assertEqual(
            total, self.manifest["summary"]["groupedOccurrenceCount"])

    def test_coverage_curve_is_monotonic_and_closes_at_one(self) -> None:
        curve = self.manifest["groupedCoverageByFamilyRank"]
        self.assertTrue(curve)
        running = [row["cumulativeGroupedOccurrences"] for row in curve]
        self.assertEqual(running, sorted(running))
        self.assertEqual(
            running[-1], self.manifest["summary"]["groupedOccurrenceCount"])
        self.assertAlmostEqual(curve[-1]["coverage"], 1.0, places=6)

    def test_admission_counts_partition_the_families(self) -> None:
        counts = self.manifest["summary"]["admissionStatusCounts"]
        self.assertEqual(
            sum(counts.values()), self.manifest["summary"]["familyCount"])

    def test_missing_evidence_root_degrades_instead_of_failing(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            absent = Path(directory) / "not-here"
            manifest = builder.build_manifest(builder.REPOSITORY_ROOT, absent)
        self.assertEqual(manifest["summary"]["familiesWithEvidence"], 0)
        self.assertEqual(
            manifest["summary"]["familyCount"],
            self.manifest["summary"]["familyCount"])
        for family in manifest["families"]:
            self.assertIn(
                "PARENT_MATERIAL_EVIDENCE_ABSENT",
                family["admission"]["blockers"])

    def test_committed_manifest_is_current(self) -> None:
        path = builder.REPOSITORY_ROOT / builder.MANIFEST_RELATIVE_PATH
        self.assertTrue(path.is_file(), f"manifest is missing: {path}")
        on_disk = json.loads(path.read_text(encoding="utf-8"))
        self.assertEqual(
            on_disk["artifactSha256"], self.manifest["artifactSha256"],
            "Data/Effects/Contracts/effect-family-manifest.v1.json is stale; "
            "rerun build_effect_family_manifest.py --mode build")


if __name__ == "__main__":
    unittest.main(verbosity=2)
