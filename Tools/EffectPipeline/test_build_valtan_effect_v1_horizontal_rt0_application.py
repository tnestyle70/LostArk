from __future__ import annotations

import copy
import json
import tempfile
import unittest
from pathlib import Path

import build_valtan_effect_v1_horizontal_rt0_application as application


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
ARTIFACT_PATH = REPOSITORY_ROOT / application.OUTPUT_RELATIVE_PATH
SCHEMA_PATH = REPOSITORY_ROOT / application.SCHEMA_RELATIVE_PATH


class ValtanEffectV1HorizontalRt0ApplicationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.document = application.build_application(REPOSITORY_ROOT)

    def _mutated(self, mutate) -> dict:
        document = copy.deepcopy(self.document)
        mutate(document)
        document.pop("artifactSha256", None)
        document["artifactSha256"] = application.canonical_sha256(document)
        return document

    def test_real_build_is_byte_current_and_closes_the_669_denominator(self) -> None:
        self.assertEqual(application.pretty_json_bytes(self.document), ARTIFACT_PATH.read_bytes())
        summary = self.document["summary"]
        self.assertEqual(summary["rowCount"], 669)
        self.assertEqual(
            summary["carrierCounts"],
            {"DECAL": 33, "MESH": 174, "PRESENTATION": 1, "RIBBON": 3, "SPRITE": 458},
        )
        self.assertEqual(
            summary["fineRendererCounts"],
            {
                "ANIM_TRAIL": 3,
                "DECAL_PARTICLE": 33,
                "LIGHT_PRESENTATION": 1,
                "MESH_PARTICLE": 173,
                "SPRITE_PARTICLE": 458,
                "STANDALONE_MESH": 1,
            },
        )
        self.assertEqual(sum(summary["applicationStateCounts"].values()), 669)
        self.assertEqual(summary["unresolvedWithoutDiagnosisCount"], 0)

    def test_all_24_product_patterns_are_reverse_joined_before_focused_review(self) -> None:
        summary = self.document["summary"]
        self.assertEqual(summary["productPatternCount"], 24)
        self.assertEqual(sum(summary["patternOccurrenceCounts"].values()), 669)
        self.assertEqual(summary["patternOccurrenceCounts"]["VALTAN_WHIRLWIND"], 12)
        self.assertEqual(
            summary["patternOccurrenceCounts"]["VALTAN_ARMOR_BREAK_OPENING"],
            10,
        )
        self.assertEqual(summary["patternOccurrenceCounts"]["VALTAN_MAGIC_CHOICE"], 19)
        self.assertNotIn("VALTAN_DASH_CHARGE", summary["patternOccurrenceCounts"])
        self.assertEqual(
            summary["patternStageOccurrenceCounts"]["VALTAN_WHIRLWIND/SPIN"],
            9,
        )
        self.assertEqual(
            summary["patternStageOccurrenceCounts"]["VALTAN_WHIRLWIND/RECOVERY"],
            3,
        )
        self.assertEqual(
            summary["patternStageOccurrenceCounts"]
            ["VALTAN_ARMOR_BREAK_OPENING/WALL_CHARGE"],
            10,
        )
        self.assertEqual(
            summary["patternStageOccurrenceCounts"]["VALTAN_MAGIC_CHOICE/INNER"],
            4,
        )
        self.assertEqual(
            summary["patternStageOccurrenceCounts"]["VALTAN_MAGIC_CHOICE/OUTER"],
            2,
        )
        self.assertEqual(
            summary["patternStageOccurrenceCounts"]["VALTAN_MAGIC_CHOICE/RECOVERY"],
            13,
        )
        for row in self.document["rows"]:
            self.assertEqual(row["compositionIds"], [row["composition"]["compositionId"]])
            self.assertTrue(row["composition"]["patternId"].startswith("VALTAN_"))

    def test_program_and_layout_denominators_preserve_fidelity(self) -> None:
        self.assertEqual(
            self.document["summary"]["programFidelityCounts"],
            {
                "BOUNDED_TRANSLATED": 1,
                "FAMILY_REPRESENTATIVE": 252,
                "NOT_APPLICABLE_PRESENTATION": 1,
                "PROJECT_RECONSTRUCTED": 276,
                "SOURCE_EXACT": 139,
            },
        )
        self.assertEqual(
            self.document["summary"]["programFidelityByCarrierCounts"]["MESH"],
            {
                "FAMILY_REPRESENTATIVE": 76,
                "PROJECT_RECONSTRUCTED": 70,
                "SOURCE_EXACT": 28,
            },
        )
        self.assertEqual(
            self.document["summary"]["layoutStatusCounts"],
            {
                "NAMED_NATIVE_WIRE_ONLY_REQUIRES_COUNT_EXTENSION": 141,
                "NAMED_NATIVE_WIRE_ONLY_WITHIN_COUNT_CAPS": 249,
                "NOT_APPLICABLE_PRESENTATION": 1,
                "SOURCE_NAMES_ONLY": 1,
                "UNRESOLVED": 277,
            },
        )

    def test_source_render_state_uses_sealed_precedence_and_keeps_unknowns(self) -> None:
        by_carrier = self.document["summary"]["sourceRenderStateByCarrierCounts"]
        self.assertEqual(
            by_carrier["SPRITE"],
            {
                "additive_one_sided_depth_off_distortion_unknown": 1,
                "additive_one_sided_depth_read_distortion_unknown": 202,
                "additive_two_sided_depth_read_distortion_unknown": 5,
                "modulate_one_sided_depth_read_distortion_unknown": 1,
                "translucent_one_sided_depth_read_distortion_unknown": 216,
                "translucent_two_sided_depth_read_distortion_unknown": 29,
                "unknown": 4,
            },
        )
        self.assertEqual(
            by_carrier["MESH"],
            {
                "additive_one_sided_depth_read_distortion_unknown": 9,
                "additive_two_sided_depth_read_distortion_unknown": 1,
                "masked_one_sided_depth_read_distortion_unknown": 35,
                "translucent_one_sided_depth_read_distortion_unknown": 62,
                "translucent_two_sided_depth_read_distortion_unknown": 63,
                "unknown": 4,
            },
        )
        self.assertEqual(by_carrier["DECAL"], {
            "translucent_one_sided_depth_read_distortion_unknown": 18,
            "unknown": 15,
        })
        self.assertEqual(self.document["summary"]["sourceCurrentComparableCount"], 645)
        self.assertEqual(self.document["summary"]["sourceCurrentMismatchCount"], 545)

    def test_two_direct_material_rows_use_strict_family_fallback(self) -> None:
        rows_by_source = {
            row["occurrence"]["sourceMaterialPath"]: row
            for row in self.document["rows"]
            if row["occurrence"]["sourceMaterialPath"]
        }
        mesh = rows_by_source["fx_m_mi_05.fx_m.fx_a_me_panning_01_ts_ad"]
        mesh_state = mesh["adapter"]["sourceRenderState"]
        self.assertIn(
            mesh_state["provenance"],
            {"PARENT_FAMILY_MANIFEST_FALLBACK", "PARENT_MISSING_FAMILY_PROPS_FALLBACK"},
        )
        self.assertEqual(mesh_state["blendMode"], "ADDITIVE")
        self.assertTrue(mesh_state["twoSided"])
        self.assertTrue(mesh_state["admissionUsable"])

        sprite = rows_by_source["fx_m_mi_05.fx_m.fx_c_pa_aura_02_tr"]
        sprite_state = sprite["adapter"]["sourceRenderState"]
        self.assertIn(
            sprite_state["provenance"],
            {"PARENT_FAMILY_MANIFEST_FALLBACK", "PARENT_MISSING_FAMILY_PROPS_FALLBACK"},
        )
        self.assertEqual(sprite_state["blendMode"], "TRANSLUCENT")
        self.assertFalse(sprite_state["twoSided"])

    def test_parent_blocked_state_is_diagnostic_and_never_bound(self) -> None:
        rows = [
            row
            for row in self.document["rows"]
            if row["occurrence"]["sourceParentBlocker"] is not None
            and row["adapter"]["sourceRenderState"]["status"] == "RESOLVED_DIAGNOSTIC_ONLY"
        ]
        self.assertGreater(len(rows), 0)
        for row in rows:
            self.assertFalse(row["adapter"]["sourceRenderState"]["admissionUsable"])
            self.assertIn("SOURCE_PARENT_RESOLUTION_BLOCKED", row["application"]["blockers"])
            self.assertIsNone(row["application"]["bindingIdentity"])

    def test_source_values_are_visible_but_unmaterialized_packets_do_not_bind(self) -> None:
        descriptor_counts = self.document["summary"]["descriptorEvidenceCounts"]
        self.assertEqual(descriptor_counts["SOURCE_CHILD_AND_PARENT_VALUES_AVAILABLE_UNPACKED"], 643)
        self.assertEqual(descriptor_counts["SOURCE_CHILD_VALUES_AVAILABLE_PARENT_DEFAULTS_MISSING"], 17)
        self.assertEqual(self.document["summary"]["bindingIdentityCount"], 0)
        self.assertEqual(
            self.document["summary"]["applicationStateCounts"],
            {"EVIDENCE_BLOCKED": 662, "FEATURE_DEFERRED": 6, "PRESENTATION_DEFERRED": 1},
        )
        for row in self.document["rows"]:
            if row["descriptor"]["packetClosure"] == "NOT_MATERIALIZED":
                self.assertIn("DESCRIPTOR_PACKET_NOT_MATERIALIZED", row["application"]["blockers"])
                self.assertIsNone(row["application"]["bindingIdentity"])

    def test_schema_is_valid_json_and_validates_when_jsonschema_is_available(self) -> None:
        schema = json.loads(SCHEMA_PATH.read_text(encoding="utf-8"))
        self.assertEqual(schema["$id"], SCHEMA_PATH.name)
        try:
            import jsonschema
        except ModuleNotFoundError:
            return
        jsonschema.Draft202012Validator.check_schema(schema)
        jsonschema.validate(self.document, schema)

    def test_check_mode_accepts_canonical_bytes_and_rejects_stale_bytes(self) -> None:
        with tempfile.TemporaryDirectory() as temporary_directory:
            output = Path(temporary_directory) / "application.json"
            output.write_bytes(application.pretty_json_bytes(self.document))
            self.assertEqual(application.main(["--check", "--output", str(output)]), 0)
            output.write_bytes(output.read_bytes() + b" ")
            self.assertEqual(application.main(["--check", "--output", str(output)]), 1)

    def test_validator_rejects_binding_on_an_evidence_blocked_row(self) -> None:
        def mutate(document: dict) -> None:
            row = next(row for row in document["rows"] if row["application"]["state"] == "EVIDENCE_BLOCKED")
            row["application"]["bindingIdentity"] = {
                "status": "CANDIDATE_NOT_PUBLISHED",
                "bindingId": "effect-binding." + "0" * 64,
            }

        with self.assertRaises(application.ApplicationError):
            application.validate_application(self._mutated(mutate))

    def test_validator_rejects_parent_blocker_admission_promotion(self) -> None:
        def mutate(document: dict) -> None:
            row = next(
                row
                for row in document["rows"]
                if row["occurrence"]["sourceParentBlocker"] is not None
            )
            row["adapter"]["sourceRenderState"]["admissionUsable"] = True

        with self.assertRaises(application.ApplicationError):
            application.validate_application(self._mutated(mutate))

    def test_validator_rejects_duplicate_occurrence_and_undiagnosed_deferred_row(self) -> None:
        def duplicate(document: dict) -> None:
            document["rows"][1]["occurrence"]["occurrenceId"] = document["rows"][0]["occurrence"]["occurrenceId"]

        with self.assertRaises(application.ApplicationError):
            application.validate_application(self._mutated(duplicate))

        def undiagnosed(document: dict) -> None:
            row = next(row for row in document["rows"] if row["application"]["state"] != "RT0_APPLICATION_CANDIDATE")
            row["application"]["blockers"] = []

        with self.assertRaises(application.ApplicationError):
            application.validate_application(self._mutated(undiagnosed))


if __name__ == "__main__":
    unittest.main()
