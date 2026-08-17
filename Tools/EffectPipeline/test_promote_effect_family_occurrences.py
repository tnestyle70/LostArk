#!/usr/bin/env python3
"""Regression tests for family occurrence promotion."""

from __future__ import annotations

import importlib.util
import unittest
from pathlib import Path

SCRIPT_PATH = Path(__file__).resolve().with_name(
    "promote_effect_family_occurrences.py")
SPEC = importlib.util.spec_from_file_location(
    "promote_effect_family_occurrences", SCRIPT_PATH)
assert SPEC is not None and SPEC.loader is not None
promoter = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(promoter)

PARENT = "fx_mastermaterial.fx_mm.fx_mm_simple_01_ad"
TARGET = "effect.ue3.simple-01.v1"


def element(**overrides):
    base = {
        "id": "e0",
        "visible": True,
        "resources": [{"slotId": "base", "assetId": "Effect/x.dds"}],
        "material": {
            "renderProfile": "additive_one_sided_depth_read",
            "sourceProfile": {
                "enabled": True,
                "parentMaterialPath": PARENT,
                "runtimeShaderProfileId": promoter.GROUPED_PROFILE,
            },
        },
    }
    material = overrides.pop("material", None)
    base.update(overrides)
    if material:
        base["material"].update(material)
    return base


def evaluate(candidate, **kwargs):
    return promoter.evaluate(
        candidate, PARENT,
        kwargs.get("carriers", {"sprite"}),
        kwargs.get("roleSets", {("base",)}),
        kwargs.get("renderProfiles", {"additive_one_sided_depth_read"}),
        TARGET)


class PromotionRuleTests(unittest.TestCase):
    def test_matching_grouped_occurrence_promotes(self) -> None:
        ok, reasons = evaluate(element())
        self.assertTrue(ok)
        self.assertEqual(reasons, [])

    def test_existing_typed_profile_is_never_overwritten(self) -> None:
        # Authored work. Promoting over it would silently revert a decision.
        ok, reasons = evaluate(element(material={"sourceProfile": {
            "enabled": True, "parentMaterialPath": PARENT,
            "runtimeShaderProfileId": "effect.ue3.slice.v1"}}))
        self.assertFalse(ok)
        self.assertIn(
            "NON_GROUPED_CURRENT_PROFILE:effect.ue3.slice.v1", reasons)

    def test_already_promoted_is_not_rewritten(self) -> None:
        ok, reasons = evaluate(element(material={"sourceProfile": {
            "enabled": True, "parentMaterialPath": PARENT,
            "runtimeShaderProfileId": TARGET}}))
        self.assertFalse(ok)
        self.assertIn("ALREADY_PROMOTED", reasons)

    def test_mesh_carrier_is_refused_for_a_sprite_only_family(self) -> None:
        ok, reasons = evaluate(element(resources=[
            {"slotId": "base"}, {"slotId": promoter.MESH_SLOT}]))
        self.assertFalse(ok)
        self.assertIn("CARRIER_NOT_ALLOWED:mesh", reasons)

    def test_extra_bound_slot_is_refused(self) -> None:
        ok, reasons = evaluate(element(resources=[
            {"slotId": "base"}, {"slotId": "mask"}]))
        self.assertFalse(ok)
        self.assertIn("ROLE_SET_NOT_ALLOWED:base+mask", reasons)

    def test_unlisted_render_profile_is_refused(self) -> None:
        ok, reasons = evaluate(element(material={
            "renderProfile": "alpha_one_sided_depth_read"}))
        self.assertFalse(ok)
        self.assertIn(
            "RENDER_PROFILE_NOT_ALLOWED:alpha_one_sided_depth_read", reasons)

    def test_execution_packet_owner_is_left_alone(self) -> None:
        ok, reasons = evaluate(element(material={
            "execution": {"enabled": True, "opcode": 15}}))
        self.assertFalse(ok)
        self.assertIn("EXECUTION_PACKET_OWNS_THIS_ELEMENT", reasons)

    def test_disabled_source_profile_is_refused(self) -> None:
        ok, reasons = evaluate(element(material={"sourceProfile": {
            "enabled": False, "parentMaterialPath": PARENT,
            "runtimeShaderProfileId": promoter.GROUPED_PROFILE}}))
        self.assertFalse(ok)
        self.assertIn("SOURCE_PROFILE_DISABLED", reasons)

    def test_other_parent_is_refused(self) -> None:
        ok, reasons = evaluate(element(material={"sourceProfile": {
            "enabled": True, "parentMaterialPath": "fx_m.other_01_tr",
            "runtimeShaderProfileId": promoter.GROUPED_PROFILE}}))
        self.assertFalse(ok)
        self.assertIn("PARENT_MISMATCH", reasons)


class CorpusReportTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.result = promoter.collect(
            PARENT, TARGET, {"sprite"}, {("base",)},
            {"additive_one_sided_depth_read",
             "additive_two_sided_depth_read"}, set())

    def test_every_occurrence_is_either_promoted_or_explained(self) -> None:
        for row in self.result["skip"]:
            self.assertTrue(
                row["reasons"], f"unexplained skip: {row['elementId']}")

    def test_report_is_read_only(self) -> None:
        before = {
            path: path.read_bytes()
            for path in promoter.AUTHORED_DIRECTORY.glob("*.effect.json")
        }
        promoter.collect(
            PARENT, TARGET, {"sprite"}, {("base",)},
            {"additive_one_sided_depth_read"}, set())
        for path, payload in before.items():
            self.assertEqual(
                path.read_bytes(), payload, f"report mutated {path.name}")

    def test_exclusion_removes_a_document_from_promotion(self) -> None:
        promoted = {row["effectAssetId"] for row in self.result["promote"]}
        self.assertTrue(promoted)
        victim = sorted(promoted)[0]
        guarded = promoter.collect(
            PARENT, TARGET, {"sprite"}, {("base",)},
            {"additive_one_sided_depth_read",
             "additive_two_sided_depth_read"}, {victim})
        self.assertNotIn(
            victim, {row["effectAssetId"] for row in guarded["promote"]})
        excluded = [row for row in guarded["skip"]
                    if row["effectAssetId"] == victim]
        self.assertTrue(excluded)
        for row in excluded:
            self.assertIn("EXCLUDED_BY_REQUEST", row["reasons"])


if __name__ == "__main__":
    unittest.main(verbosity=2)
