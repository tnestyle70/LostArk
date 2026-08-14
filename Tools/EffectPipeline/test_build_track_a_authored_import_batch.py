#!/usr/bin/env python3
"""Contract tests for the first four-class Track A authored import batch."""

from __future__ import annotations

import copy
import importlib.util
import json
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock


SCRIPT_PATH = Path(__file__).resolve().with_name(
    "build_track_a_authored_import_batch.py"
)
SPEC = importlib.util.spec_from_file_location(
    "build_track_a_authored_import_batch", SCRIPT_PATH
)
assert SPEC is not None and SPEC.loader is not None
builder = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = builder
SPEC.loader.exec_module(builder)


def reseal(batch: dict) -> None:
    batch.pop("artifactSha256", None)
    batch["artifactSha256"] = builder.canonical_json_sha256(batch)


class TrackAAuthoredImportBatchTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.repository_root = builder.REPOSITORY_ROOT
        cls.batch = builder.build_batch(cls.repository_root)

    def test_schema_is_closed_and_material_disposition_is_exact_one_of(self) -> None:
        schema = builder.load_json(
            self.repository_root / builder.SCHEMA_RELATIVE_PATH
        )
        self.assertFalse(schema["additionalProperties"])
        material = schema["$defs"]["materialDisposition"]
        self.assertEqual(len(material["oneOf"]), 3)
        branches = [
            schema["$defs"][reference["$ref"].rsplit("/", 1)[1]]
            for reference in material["oneOf"]
        ]
        self.assertEqual(
            {
                branch["properties"]["kind"]["const"]
                for branch in branches
            },
            {
                "TYPED_EXECUTION",
                "ADMITTED_SOURCE_PROFILE",
                "FAIL_CLOSED",
            },
        )
        self.assertTrue(all(not branch["additionalProperties"] for branch in branches))
        self.assertEqual(
            schema["$defs"]["denominators"]["properties"]["stageCount"]["const"],
            13,
        )
        self.assertEqual(
            schema["$defs"]["denominators"]["properties"]["elementPlanCount"]["const"],
            79,
        )
        self.assertEqual(
            schema["$defs"]["fullScopeDenominators"]["properties"][
                "fullStageCount"
            ]["const"],
            74,
        )
        self.assertEqual(
            schema["properties"]["legacyStarterCandidates"]["minItems"], 88
        )

    def test_exact_scope_denominators_and_no_product_mutation(self) -> None:
        batch = self.batch
        self.assertEqual(batch["denominators"], builder.EXPECTED_DENOMINATORS)
        self.assertEqual(
            batch["materialDispositionCounts"],
            {
                "ADMITTED_SOURCE_PROFILE": 16,
                "FAIL_CLOSED": 63,
                "TYPED_EXECUTION": 0,
            },
        )
        self.assertEqual(
            batch["carrierDispositionCounts"],
            {
                "FAMILY_ADAPTER_REQUIRED": 4,
                "GENERIC_PARTICLE_IMPORT_CANDIDATE": 71,
                "SUPPLEMENTAL_ADAPTER_PRESERVE": 4,
            },
        )
        self.assertFalse(batch["scope"]["productCatalogMutation"])
        self.assertFalse(batch["scope"]["animationEventMutation"])
        self.assertFalse(batch["admission"]["productMappingMutation"])
        self.assertFalse(batch["admission"]["visualApproval"])
        self.assertEqual(
            batch["fullScopeDenominators"],
            builder.EXPECTED_FULL_SCOPE_DENOMINATORS,
        )
        self.assertEqual(len(batch["legacyStarterStages"]), 61)
        self.assertEqual(len(batch["legacyStarterCandidates"]), 88)
        self.assertEqual(len(batch["scope"]["unifiedCandidateEffectAssetIds"]), 101)
        self.assertTrue(all(not row["productMutation"] for row in batch["stages"]))
        self.assertTrue(all(not row["productMutation"] for row in batch["elementPlans"]))
        builder.validate_batch(batch, self.repository_root)

    def test_track_a_program_to_skill_stage_mapping_is_exact_and_f_is_excluded(self) -> None:
        track_a = [
            row
            for row in self.batch["stages"]
            if row["mode"] == builder.TRACK_A_SELECTION_KIND
        ]
        self.assertEqual(
            [
                (
                    row["characterClass"],
                    row["skillId"],
                    row["stageIndex"],
                    row["clip"],
                    row["target"]["effectAssetId"],
                )
                for row in track_a
            ],
            [
                ("ARTIST", 31000, 0, "sdm_att_battle_1_03", "effect.artist.skill.31000.ba1.unified"),
                ("ARTIST", 31000, 1, "sdm_att_battle_1_02", "effect.artist.skill.31000.ba2.unified"),
                ("ARTIST", 31000, 2, "sdm_att_battle_1_01", "effect.artist.skill.31000.ba3.unified"),
                ("ARTIST", 31000, 3, "sdm_att_battle_1_04", "effect.artist.skill.31000.ba4.unified"),
                ("DIMENSIONMASTER", 2050010, 0, "pc_sp_m_00_sk_att_battle_1_01", "effect.dimensionmaster.skill.2050010.ba1.unified"),
                ("DIMENSIONMASTER", 2050010, 1, "pc_sp_m_00_sk_att_battle_1_02", "effect.dimensionmaster.skill.2050010.ba2.unified"),
                ("DIMENSIONMASTER", 2050010, 2, "pc_sp_m_00_sk_att_battle_1_03", "effect.dimensionmaster.skill.2050010.ba3.unified"),
                ("DIMENSIONMASTER", 2050010, 3, "pc_sp_m_00_sk_att_battle_1_04", "effect.dimensionmaster.skill.2050010.ba4.unified"),
                ("LANCE_MASTER", 34010, 0, "flm_att_identity1_1_01", "effect.lancemaster.skill.34010.ba1.unified"),
                ("LANCE_MASTER", 34010, 1, "flm_att_identity1_1_02", "effect.lancemaster.skill.34010.ba2.unified"),
                ("LANCE_MASTER", 34010, 2, "flm_att_identity1_1_03", "effect.lancemaster.skill.34010.ba3.unified"),
                ("LANCE_MASTER", 34010, 3, "flm_att_identity1_1_04", "effect.lancemaster.skill.34010.ba4.unified"),
            ],
        )
        serialized = json.dumps(
            {
                "stages": self.batch["stages"],
                "legacyStarterStages": self.batch["legacyStarterStages"],
                "legacyStarterCandidates": self.batch["legacyStarterCandidates"],
            },
            ensure_ascii=False,
        )
        self.assertNotIn("effect.artist.skill.31470", serialized)
        self.assertTrue(self.batch["scope"]["artistFExcluded"])
        self.assertEqual(
            self.batch["scope"]["artistFDirectSliceEffectAssetId"],
            "effect.artist.skill.31470.unified",
        )

    def test_warlord_canary_uses_exact_five_resource_backed_stage_zero_carriers(self) -> None:
        stage = next(
            row
            for row in self.batch["stages"]
            if row["mode"] == builder.WARLORD_SELECTION_KIND
        )
        self.assertEqual(
            (
                stage["characterClass"],
                stage["skillId"],
                stage["stageIndex"],
                stage["clip"],
                stage["selection"]["recordId"],
            ),
            (
                "WARLORD",
                17000,
                0,
                "wgl_att_battle_1_01",
                builder.WARLORD_CANARY_ID,
            ),
        )
        plans = [
            row for row in self.batch["elementPlans"]
            if row["stageKey"] == stage["stageKey"]
        ]
        self.assertEqual(
            {row["source"]["elementId"] for row in plans},
            {
                "fx_pc_wgl_06.par_o_wgl_normalatk_01_01.particlespriteemitter_0",
                "fx_pc_wgl_06.par_o_wgl_normalatk_01_01.particlespriteemitter_25",
                "fx_pc_wgl_06.par_o_wgl_normalatk_01_01.particlespriteemitter_26",
                "fx_pc_wgl_06.par_o_wgl_normalatk_01_01.particlespriteemitter_27",
                "fx_pc_wgl_06.par_o_wgl_normalatk_01_01.particlespriteemitter_81",
            },
        )
        self.assertTrue(
            all(row["source"]["sourceEventId"] == "source-event-003" for row in plans)
        )
        self.assertTrue(
            all(row["materialDisposition"]["kind"] == "FAIL_CLOSED" for row in plans)
        )
        self.assertTrue(
            all(
                "WARLORD_CANARY_MATERIAL_FAIL_CLOSED"
                in row["materialDisposition"]["blockers"]
                for row in plans
            )
        )
        self.assertTrue(
            all(
                row["carrierDisposition"]["kind"]
                == "GENERIC_PARTICLE_IMPORT_CANDIDATE"
                for row in plans
            )
        )

    def test_sources_and_legacy_rollback_baselines_are_hash_pinned(self) -> None:
        artifacts = {
            row["path"]: row for row in self.batch["inputArtifacts"]
        }
        self.assertGreaterEqual(len(artifacts), 136)
        for stage in self.batch["stages"]:
            for key in (
                "manifest",
                "sourceReceipt",
                "importedDocument",
                "conversionReceipt",
            ):
                reference = stage["sourceArtifacts"][key]
                registered = artifacts[reference["path"]]
                self.assertEqual(reference["rawSha256"], registered["rawSha256"])
                self.assertEqual(
                    reference["canonicalJsonSha256"],
                    registered["canonicalJsonSha256"],
                )
            baseline = stage["target"]["legacyRollbackBaseline"]
            self.assertEqual(
                baseline["policy"], "IMMUTABLE_LEGACY_ROLLBACK_EXACT"
            )
            baseline_artifact = artifacts[baseline["path"]]
            self.assertEqual(baseline["rawSha256"], baseline_artifact["rawSha256"])
            self.assertEqual(
                baseline["canonicalJsonSha256"],
                baseline_artifact["canonicalJsonSha256"],
            )
            self.assertEqual(stage["target"]["requiredOutputVersion"], 13)
            self.assertEqual(
                stage["target"]["effectAssetId"],
                f'{baseline["effectAssetId"]}.unified',
            )
            self.assertEqual(
                stage["target"]["path"],
                "Data/Effects/Authored/"
                f'{stage["target"]["effectAssetId"]}.effect.json',
            )
            candidate = stage["target"]["candidateBaseline"]
            self.assertEqual(candidate["policy"], "EXPECTED_EXACT_OR_REFUSE")
            candidate_artifact = artifacts[stage["target"]["path"]]
            self.assertEqual(
                candidate["expectedRawSha256"], candidate_artifact["rawSha256"]
            )
            self.assertEqual(
                candidate["expectedCanonicalJsonSha256"],
                candidate_artifact["canonicalJsonSha256"],
            )
            self.assertEqual(candidate["authoringVersion"], 13)
            self.assertTrue(
                (self.repository_root / stage["target"]["path"]).is_file()
            )

    def test_remaining_stage_clip_and_candidate_partition_is_exact(self) -> None:
        stages = self.batch["legacyStarterStages"]
        candidates = self.batch["legacyStarterCandidates"]
        clips = [clip for stage in stages for clip in stage["clips"]]
        self.assertEqual(len(clips), 100)
        self.assertEqual(
            sum(clip["status"] == "visualBearing" for clip in clips), 89
        )
        self.assertEqual(
            sum(clip["status"] != "visualBearing" for clip in clips), 11
        )
        references = [
            clip["candidateEffectAssetId"]
            for clip in clips
            if clip["candidateEffectAssetId"] is not None
        ]
        self.assertEqual(len(references), 89)
        self.assertEqual(len(set(references)), 88)
        repeated = {item for item in references if references.count(item) > 1}
        self.assertEqual(repeated, {"effect.artist.skill.31210.ba1.unified"})
        self.assertEqual(
            {row["target"]["effectAssetId"] for row in candidates},
            set(references),
        )

    def test_legacy_candidates_are_exact_pinned_and_never_product_mapped(self) -> None:
        artifacts = {row["path"]: row for row in self.batch["inputArtifacts"]}
        catalog_candidate_ids = {
            row["effectAssetId"]
            for row in builder.load_json(
                self.repository_root / builder.EFFECT_CATALOG_RELATIVE_PATH
            )["effects"]
        }
        existing = []
        orphaned = []
        eol_only = []
        for row in self.batch["legacyStarterCandidates"]:
            legacy = row["legacyRollbackBaseline"]
            starter = row["starterSource"]
            target = row["target"]
            self.assertEqual(legacy["authoringVersion"], 12)
            self.assertEqual(legacy["rawSha256"], artifacts[legacy["path"]]["rawSha256"])
            self.assertEqual(starter["rawSha256"], artifacts[starter["path"]]["rawSha256"])
            self.assertFalse(row["trackAAdmission"])
            self.assertFalse(row["productMutation"])
            self.assertFalse(row["visualApproval"])
            self.assertNotIn(target["effectAssetId"], catalog_candidate_ids)
            if target["candidateBaseline"]["policy"] == "EXPECTED_EXACT_OR_REFUSE":
                existing.append(row)
                self.assertEqual(starter["kind"], "EXISTING_UNIFIED_CANDIDATE")
                self.assertEqual(starter["path"], target["path"])
            if row["productReference"]["orphanedCatalogReference"]:
                orphaned.append(legacy["effectAssetId"])
            if legacy["rolloutHashDisposition"] == "EOL_NORMALIZED_MATCH":
                eol_only.append(legacy["effectAssetId"])
        self.assertGreaterEqual(len(existing), 1)
        dm_existing = next(
            row
            for row in existing
            if row["target"]["effectAssetId"]
            == "effect.dimensionmaster.skill.2050500.unified"
        )
        self.assertEqual(dm_existing["starterSource"]["kind"], "EXISTING_UNIFIED_CANDIDATE")
        self.assertIn(dm_existing["starterSource"]["authoringVersion"], {12, 13})
        self.assertEqual(set(orphaned), builder.EXPECTED_ORPHANED_PRODUCT_EFFECT_IDS)
        self.assertEqual(len(eol_only), 17)

    def test_candidate_normalization_is_collision_free_and_exact(self) -> None:
        self.assertEqual(
            builder._unified_candidate_identity(
                "effect.artist.skill.31200.authored-baseline"
            )[0],
            "effect.artist.skill.31200.unified",
        )
        self.assertEqual(
            builder._unified_candidate_identity(
                "effect.warlord.skill.17820.authored-baseline.clip1"
            )[0],
            "effect.warlord.skill.17820.clip1.unified",
        )
        self.assertEqual(
            builder._unified_candidate_identity("effect.lancemaster.skill.34140.ba1")[0],
            "effect.lancemaster.skill.34140.ba1.unified",
        )
        all_ids = self.batch["scope"]["unifiedCandidateEffectAssetIds"]
        self.assertEqual(len(all_ids), len(set(all_ids)), 101)

    def test_material_disposition_rows_are_mutually_exclusive(self) -> None:
        for row in self.batch["elementPlans"]:
            material = row["materialDisposition"]
            if material["kind"] == "ADMITTED_SOURCE_PROFILE":
                self.assertEqual(
                    set(material),
                    {
                        "kind",
                        "sourceProfileSha256",
                        "profileId",
                        "runtimeShaderProfileId",
                    },
                )
            elif material["kind"] == "FAIL_CLOSED":
                self.assertEqual(set(material), {"kind", "blockers"})
                self.assertTrue(material["blockers"])
            else:
                self.assertEqual(material["kind"], "TYPED_EXECUTION")
                self.assertEqual(set(material), {"kind", "executionSnapshotSha256"})

    def test_build_is_byte_deterministic_and_tracked_output_is_current(self) -> None:
        rebuilt = builder.build_batch(self.repository_root)
        self.assertEqual(
            builder.pretty_json_bytes(self.batch),
            builder.pretty_json_bytes(rebuilt),
        )
        output = self.repository_root / builder.DEFAULT_OUTPUT_RELATIVE_PATH
        self.assertTrue(output.is_file())
        self.assertEqual(output.read_bytes(), builder.pretty_json_bytes(self.batch))

    def assert_invalid_batch_preserves_previous_output(
        self,
        mutator,
        expected_error: str,
    ) -> None:
        changed = copy.deepcopy(self.batch)
        mutator(changed)
        reseal(changed)
        with tempfile.TemporaryDirectory() as temporary:
            output = Path(temporary) / "batch.json"
            previous = b'{"previous":"validated"}\n'
            output.write_bytes(previous)
            with mock.patch.object(builder, "build_batch", return_value=changed):
                with self.assertRaisesRegex(builder.ContractError, expected_error):
                    builder.build_and_write(self.repository_root, output)
            self.assertEqual(output.read_bytes(), previous)
            self.assertEqual(list(output.parent.glob("*.tmp")), [])

    def test_duplicate_target_and_unsafe_path_fail_transactionally(self) -> None:
        self.assert_invalid_batch_preserves_previous_output(
            lambda value: value["elementPlans"].append(
                copy.deepcopy(value["elementPlans"][0])
            ),
            "duplicate element plan ID",
        )
        self.assert_invalid_batch_preserves_previous_output(
            lambda value: value["stages"][0]["target"].__setitem__(
                "path", "../escape.effect.json"
            ),
            "unsafe segment",
        )

    def test_stale_receipt_and_baseline_hash_fail_transactionally(self) -> None:
        self.assert_invalid_batch_preserves_previous_output(
            lambda value: value["stages"][0]["sourceArtifacts"][
                "sourceReceipt"
            ].__setitem__("rawSha256", "0" * 64),
            "stage artifact identity mismatch",
        )
        self.assert_invalid_batch_preserves_previous_output(
            lambda value: value["stages"][0]["target"][
                "legacyRollbackBaseline"
            ].__setitem__(
                "rawSha256", "f" * 64
            ),
            "legacy rollback baseline changed",
        )

    def test_candidate_must_not_exist_or_match_exact_baseline(self) -> None:
        def stale_candidate_hash(value: dict) -> None:
            candidate = value["stages"][0]["target"]["candidateBaseline"]
            candidate["expectedRawSha256"] = "a" * 64

        self.assert_invalid_batch_preserves_previous_output(
            stale_candidate_hash,
            "unified candidate baseline changed",
        )

        def pretend_existing_candidate_must_not_exist(value: dict) -> None:
            candidate = value["stages"][0]["target"]["candidateBaseline"]
            candidate["policy"] = "MUST_NOT_EXIST"
            candidate["expectedRawSha256"] = None
            candidate["expectedCanonicalJsonSha256"] = None
            candidate["authoringVersion"] = None

        self.assert_invalid_batch_preserves_previous_output(
            pretend_existing_candidate_must_not_exist,
            "unified candidate must not exist",
        )

    def test_material_one_of_violation_fails_transactionally(self) -> None:
        def mutate(value: dict) -> None:
            fail_closed = next(
                row
                for row in value["elementPlans"]
                if row["materialDisposition"]["kind"] == "FAIL_CLOSED"
            )
            fail_closed["materialDisposition"]["sourceProfileSha256"] = "a" * 64

        self.assert_invalid_batch_preserves_previous_output(
            mutate,
            "materialDisposition fields mismatch",
        )

    def test_legacy_starter_drift_duplicate_and_silent_candidate_fail(self) -> None:
        self.assert_invalid_batch_preserves_previous_output(
            lambda value: value["legacyStarterCandidates"][0][
                "legacyRollbackBaseline"
            ].__setitem__("rawSha256", "0" * 64),
            "legacy starter artifact identity mismatch",
        )

        def duplicate_target(value: dict) -> None:
            value["legacyStarterCandidates"][1]["candidateKey"] = value[
                "legacyStarterCandidates"
            ][0]["candidateKey"]

        self.assert_invalid_batch_preserves_previous_output(
            duplicate_target,
            "duplicate legacy starter candidate ID",
        )

        def give_silent_clip_a_candidate(value: dict) -> None:
            silent = next(
                clip
                for stage in value["legacyStarterStages"]
                for clip in stage["clips"]
                if clip["status"] != "visualBearing"
            )
            silent["candidateEffectAssetId"] = value["legacyStarterCandidates"][0][
                "candidateKey"
            ]

        self.assert_invalid_batch_preserves_previous_output(
            give_silent_clip_a_candidate,
            "silent/no-carrier clip gained a candidate",
        )


if __name__ == "__main__":
    unittest.main()
