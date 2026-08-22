from __future__ import annotations

import copy
import importlib.util
import io
import json
import tempfile
import unittest
from collections import Counter
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from unittest import mock


SCRIPT = Path(__file__).with_name(
    "build_four_character_effect_v1_horizontal_application.py"
)
SCHEMA_PATH = (
    SCRIPT.parent
    / "Schemas"
    / "lostark.four-character-effect-v1-horizontal-application.schema.json"
)
SPEC = importlib.util.spec_from_file_location(
    "four_character_effect_v1_horizontal_application",
    SCRIPT,
)
assert SPEC is not None and SPEC.loader is not None
APPLICATION = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(APPLICATION)


class FourCharacterEffectV1HorizontalApplicationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.document = APPLICATION.build_application()
        cls.payload = APPLICATION.pretty_json_bytes(cls.document)
        cls.source_contract = APPLICATION.load_json(
            APPLICATION.SOURCE_CONTRACT_PATH,
            "test source contract",
        )
        cls.source_by_path = {
            row["sourceMaterialPath"]: row
            for row in cls.source_contract["materialIdentities"]
        }

    def test_product_denominator_and_taxonomy_are_complete(self) -> None:
        summary = self.document["summary"]
        self.assertEqual(1885, summary["productOccurrenceCount"])
        self.assertEqual(99, summary["productAssetCount"])
        self.assertEqual(1871, summary["horizontalFineOccurrenceCount"])
        self.assertEqual(14, summary["featureDeferredOccurrenceCount"])
        self.assertEqual(
            {
                "Artist": 377,
                "DimensionMaster": 270,
                "LanceMaster": 775,
                "Warlord": 463,
            },
            summary["domainOccurrenceCounts"],
        )
        self.assertEqual(
            {
                "AUTHORED_LEGACY_TRAIL": 8,
                "CASCADE_RIBBON": 4,
                "DECAL_PARTICLE": 41,
                "LEGACY_STANDALONE_SPRITE": 2,
                "MESH_PARTICLE": 493,
                "SPRITE_PARTICLE": 1337,
            },
            summary["fineRendererKindCounts"],
        )
        self.assertEqual(
            {
                "DECAL": 41,
                "MESH": 493,
                "RIBBON": 12,
                "SPRITE": 1339,
            },
            summary["coarseCarrierCounts"],
        )
        for field in (
            "domainOccurrenceCounts",
            "fineRendererKindCounts",
            "coarseCarrierCounts",
            "applicationStateCounts",
            "programStatusCounts",
            "layoutStatusCounts",
            "descriptorStatusCounts",
            "adapterStatusCounts",
            "sourceIdentityJoinStatusCounts",
            "sourceParentAgreementCounts",
            "sourceRenderStateCategoryCounts",
            "currentRenderProfileCounts",
            "renderStateComparisonStatusCounts",
            "manualReviewCounts",
        ):
            self.assertEqual(1885, sum(summary[field].values()), field)

    def test_application_state_partition_is_total_and_evidence_preserving(self) -> None:
        self.assertEqual(
            {
                "CURRENT_BOUND_INLINE_EXACT": 1,
                "EVIDENCE_BLOCKED": 456,
                "FEATURE_DEFERRED": 14,
                "INLINE_MIRROR_CANDIDATE": 43,
                "PROJECT_RECONSTRUCTION_PENDING": 711,
                "SOURCE_EXACT_PACKET_PENDING": 596,
                "SOURCE_EXACT_SIMPLE_RT0_PACKET_PENDING": 64,
            },
            self.document["summary"]["applicationStateCounts"],
        )
        deferred = [
            row
            for row in self.document["occurrences"]
            if row["applicationState"] == "FEATURE_DEFERRED"
        ]
        self.assertEqual(14, len(deferred))
        self.assertEqual(
            {
                "AUTHORED_LEGACY_TRAIL": 8,
                "CASCADE_RIBBON": 4,
                "LEGACY_STANDALONE_SPRITE": 2,
            },
            dict(sorted(Counter(
                row["fineRendererKind"] for row in deferred
            ).items())),
        )
        simple = [
            row
            for row in self.document["occurrences"]
            if row["applicationState"]
            == "SOURCE_EXACT_SIMPLE_RT0_PACKET_PENDING"
        ]
        self.assertEqual(64, len(simple))
        self.assertEqual({"SPRITE_PARTICLE"}, {row["fineRendererKind"] for row in simple})
        for row in simple:
            self.assertEqual("DXBC_OCCURRENCE_EXACT", row["axes"]["program"]["status"])
            self.assertNotIn(
                "OUTPUT_TOPOLOGY_MRT_UNPROVEN",
                row["blockers"],
            )
            self.assertNotIn("SCENE_INPUTS_UNPROVEN", row["blockers"])
            self.assertNotIn("WPO_VERTEX_PROGRAM_UNPROVEN", row["blockers"])

    def test_registry_binding_is_exact_and_all_other_identities_are_null(self) -> None:
        validated_registry = APPLICATION._build_validated_material_registry()
        self.assertEqual(
            APPLICATION.canonical_sha256(validated_registry),
            self.document["inputs"]["materialProgramRegistry"][
                "validatedRegistrySha256"
            ],
        )
        bound = [
            row for row in self.document["occurrences"]
            if row["bindingIdentity"] is not None
        ]
        self.assertEqual(1, len(bound))
        row = bound[0]
        self.assertEqual("effect.artist.skill.31470.unified", row["effectAssetId"])
        self.assertEqual("sprite.2b3dc6842507e910", row["elementId"])
        self.assertEqual("CURRENT_BOUND_INLINE_EXACT", row["applicationState"])
        self.assertEqual(
            "REGISTRY_BINDING_INLINE_PACKET_EXACT",
            row["runtimeProofStatus"],
        )
        self.assertEqual(
            1884,
            sum(
                occurrence["bindingIdentity"] is None
                for occurrence in self.document["occurrences"]
            ),
        )
        self.assertEqual(1, self.document["summary"]["targetRegistryBindingCount"])
        self.assertEqual(0, self.document["summary"]["registryReverseJoinMissingCount"])

    def test_merged_registry_change_is_stale_even_when_base_file_is_unchanged(self) -> None:
        merged_registry = copy.deepcopy(
            APPLICATION._build_validated_material_registry()
        )
        foreign_binding = copy.deepcopy(merged_registry["bindings"][0])
        foreign_binding["effectAssetId"] = "effect.valtan.pattern.synthetic"
        foreign_binding["elementId"] = "synthetic.fragment-binding"
        merged_registry["bindings"].append(foreign_binding)

        with mock.patch.object(
            APPLICATION.material_registry,
            "build_registry",
            return_value=merged_registry,
        ):
            merged_document = APPLICATION.build_application()
            base_input = self.document["inputs"]["materialProgramRegistry"]
            merged_input = merged_document["inputs"]["materialProgramRegistry"]
            self.assertEqual(base_input["fileSha256"], merged_input["fileSha256"])
            self.assertNotEqual(
                base_input["validatedRegistrySha256"],
                merged_input["validatedRegistrySha256"],
            )
            self.assertEqual(2, merged_input["bindingCount"])
            with tempfile.TemporaryDirectory() as temporary_directory:
                output = Path(temporary_directory) / "application.json"
                output.write_bytes(self.payload)
                with redirect_stdout(io.StringIO()), redirect_stderr(io.StringIO()):
                    self.assertEqual(
                        1,
                        APPLICATION.run(output, True, merged_document),
                    )

    def test_source_contract_join_is_exact_key_only(self) -> None:
        joined = 0
        missing = 0
        divergent = 0
        for row in self.document["occurrences"]:
            source = row["sourceMaterial"]
            source_path = source["sourceMaterialPath"]
            identity = self.source_by_path.get(source_path)
            if identity is None:
                missing += 1
                self.assertEqual(
                    "NO_COMMITTED_SOURCE_IDENTITY",
                    source["contractJoinStatus"],
                )
                self.assertIsNone(source["contractParentMaterialPath"])
                self.assertIsNone(source["parentAgreement"])
            else:
                joined += 1
                self.assertEqual(
                    "EXACT_SOURCE_MATERIAL_IDENTITY",
                    source["contractJoinStatus"],
                )
                self.assertEqual(source_path, identity["sourceMaterialPath"])
                self.assertEqual(
                    identity["parentMaterialPath"],
                    source["contractParentMaterialPath"],
                )
                expected_agreement = (
                    identity["parentMaterialPath"]
                    == source["inventoryEffectiveParentMaterialPath"]
                )
                self.assertIs(expected_agreement, source["parentAgreement"])
                divergent += not expected_agreement
        self.assertEqual(1512, joined)
        self.assertEqual(373, missing)
        self.assertEqual(63, divergent)

    def test_render_state_comparison_never_invents_depth_write(self) -> None:
        summary = self.document["summary"]
        self.assertEqual(1204, summary["sourceRenderStateResolvedCount"])
        self.assertEqual(681, summary["sourceRenderStateUnresolvedCount"])
        self.assertEqual(
            {
                "MATCH_AVAILABLE_AXES": 1125,
                "MISMATCH_AVAILABLE_AXES": 79,
                "SOURCE_STATE_UNRESOLVED": 681,
            },
            summary["renderStateComparisonStatusCounts"],
        )
        self.assertEqual(
            {"blendMode": 54, "depthTest": 20, "twoSided": 34},
            summary["renderStateMismatchAxisCounts"],
        )
        masked_count = 0
        for row in self.document["occurrences"]:
            render_state = row["renderState"]
            self.assertFalse(render_state["comparison"]["depthWriteCompared"])
            source = render_state["source"]
            if source is None:
                continue
            self.assertIsNone(source["depthWrite"])
            self.assertEqual("NOT_CAPTURED", source["depthWriteEvidence"])
            if source["blendMode"] == "BLEND_Masked":
                masked_count += 1
                self.assertIn("MASKED_DEPTH_WRITE_UNRESOLVED", row["blockers"])
        self.assertEqual(53, masked_count)

    def test_all_rows_remain_pending_manual_review(self) -> None:
        self.assertEqual(
            {"PENDING": 1885},
            self.document["summary"]["manualReviewCounts"],
        )
        self.assertTrue(
            all(
                row["manualReview"] == "PENDING"
                for row in self.document["occurrences"]
            )
        )
        self.assertFalse(
            self.document["policy"]["trackBTupleInventoryRuntimeProofAllowed"]
        )
        self.assertFalse(
            self.document["policy"]["trackBArtifactPromotedToRuntimeProof"]
        )
        self.assertFalse(self.document["inputs"]["tupleInventory"]["checkedInArtifactUsed"])

    def test_hashes_order_and_checked_in_payload_are_deterministic(self) -> None:
        APPLICATION.validate_application(self.document)
        self.assertEqual(
            self.payload,
            APPLICATION.OUTPUT_PATH.read_bytes(),
        )
        keys = [
            (row["domain"], row["effectAssetId"], row["elementOrder"], row["elementId"])
            for row in self.document["occurrences"]
        ]
        self.assertEqual(sorted(keys), keys)
        self.assertEqual(len(keys), len(set(
            (row["effectAssetId"], row["elementId"])
            for row in self.document["occurrences"]
        )))

    def test_check_mode_rejects_stale_bytes(self) -> None:
        with tempfile.TemporaryDirectory() as temporary_directory:
            output = Path(temporary_directory) / "application.json"
            output.write_bytes(self.payload)
            with redirect_stdout(io.StringIO()), redirect_stderr(io.StringIO()):
                self.assertEqual(0, APPLICATION.run(output, True, self.document))
            output.write_bytes(self.payload + b" ")
            with redirect_stdout(io.StringIO()), redirect_stderr(io.StringIO()):
                self.assertEqual(1, APPLICATION.run(output, True, self.document))

    def test_validator_rejects_duplicates_and_track_b_promotion(self) -> None:
        duplicate = copy.deepcopy(self.document)
        duplicate["occurrences"][1] = copy.deepcopy(duplicate["occurrences"][0])
        with self.assertRaisesRegex(APPLICATION.ApplicationError, "duplicate"):
            APPLICATION.validate_application(duplicate)

        promoted = copy.deepcopy(self.document)
        promoted["policy"]["trackBArtifactPromotedToRuntimeProof"] = True
        with self.assertRaisesRegex(APPLICATION.ApplicationError, "Track B"):
            APPLICATION.validate_application(promoted)

    def test_json_schema_accepts_only_the_closed_contract(self) -> None:
        schema = json.loads(SCHEMA_PATH.read_text(encoding="utf-8"))
        self.assertEqual(
            "https://json-schema.org/draft/2020-12/schema",
            schema["$schema"],
        )
        refs: list[str] = []

        def collect_refs(value: object) -> None:
            if isinstance(value, dict):
                ref = value.get("$ref")
                if isinstance(ref, str):
                    refs.append(ref)
                for child in value.values():
                    collect_refs(child)
            elif isinstance(value, list):
                for child in value:
                    collect_refs(child)

        collect_refs(schema)
        for ref in refs:
            self.assertTrue(ref.startswith("#/$defs/"), ref)
            self.assertIn(ref.removeprefix("#/$defs/"), schema["$defs"])
        APPLICATION.validate_application(self.document)
        try:
            import jsonschema
        except ImportError:
            return
        jsonschema.Draft202012Validator.check_schema(schema)
        jsonschema.Draft202012Validator(schema).validate(self.document)


if __name__ == "__main__":
    unittest.main()
