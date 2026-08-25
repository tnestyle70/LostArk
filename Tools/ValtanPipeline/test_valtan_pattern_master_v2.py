#!/usr/bin/env python3
from __future__ import annotations

import argparse
import copy
import json
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import valtan_tuning_pipeline as pipeline


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]


class ValtanPatternMasterV2Tests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.root = REPOSITORY_ROOT.resolve()
        cls.docs = pipeline.load_pipeline_documents(cls.root)
        cls.migration_docs = pipeline.load_pipeline_documents(
            cls.root,
            include_split_authoring=False,
            include_migration_fixture=True,
        )
        cls.source_manifest = pipeline.source_manifest(cls.root)

    def migrate(self):
        return pipeline.migrate_v1_to_v2(
            self.root, copy.deepcopy(self.migration_docs)
        )

    def create_directory_reparse(self, link: Path, target: Path) -> None:
        if os.name == "nt":
            completed = subprocess.run(
                [
                    "powershell",
                    "-NoProfile",
                    "-Command",
                    "New-Item",
                    "-ItemType",
                    "Junction",
                    "-Path",
                    str(link),
                    "-Target",
                    str(target),
                ],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                check=False,
            )
            self.assertEqual(0, completed.returncode, "failed to create test junction")
        else:
            link.symlink_to(target, target_is_directory=True)
        self.assertTrue(pipeline._is_reparse_point(link))

    @staticmethod
    def remove_directory_reparse(link: Path) -> None:
        if not (link.exists() or pipeline._is_reparse_point(link)):
            return
        if os.name == "nt":
            os.rmdir(link)
        else:
            link.unlink()

    def draft_patch(self):
        migrated = self.migrate()
        spin = next(
            stage
            for pattern in migrated["patterns"]
            if pattern["patternId"] == "VALTAN_WHIRLWIND"
            for stage in pattern["stages"]
            if stage["stageId"] == "SPIN"
        )
        edited_hit = copy.deepcopy(spin["hit"])
        edited_hit["shape"]["outerRadiusM"] = 10.5
        edited_hit["schedule"] = {
            "kind": "EXPLICIT_OFFSETS",
            "offsetsMs": [0, 300, 600, 900],
        }
        edited_hit["serverDamageProfileId"] = "damage.valtan.circular-spin"
        edited_hit["pushRangeM"] = 0.5
        edited_hit["pushMs"] = 75
        edited_hit["knockdown"] = True
        edited_hit["downMs"] = 500
        return {
            "schema": pipeline.DRAFT_PATCH_SCHEMA,
            "formatVersion": 1,
            "sourceRevision": self.source_manifest["sourceManifestId"],
            "operations": [
                {
                    "op": "SET_PATTERN_WEIGHT",
                    "selectionSetId": "selectionset.valtan.160.130",
                    "patternId": "VALTAN_WHIRLWIND",
                    "value": 25,
                },
                {
                    "op": "SET_PATTERN_WEIGHT",
                    "selectionSetId": "selectionset.valtan.130.109",
                    "patternId": "VALTAN_WHIRLWIND",
                    "value": 25,
                },
                {
                    "op": "SET_PATTERN_REPEAT_LIMIT",
                    "patternId": "VALTAN_WHIRLWIND",
                    "value": 3,
                },
                {
                    "op": "SET_PATTERN_RANGE",
                    "patternId": "VALTAN_WHIRLWIND",
                    "minimumRangeM": 1.0,
                    "maximumRangeM": 13.0,
                },
                {
                    "op": "SET_STAGE_DURATION",
                    "patternId": "VALTAN_WHIRLWIND",
                    "stageId": "WINDUP",
                    "durationMs": 1400,
                },
                {
                    "op": "SET_STAGE_HIT",
                    "patternId": "VALTAN_WHIRLWIND",
                    "stageId": "SPIN",
                    "hit": edited_hit,
                },
                {
                    "op": "SET_AXE_VOLLEY",
                    "patternId": "VALTAN_HIGH_JUMP",
                    "stageId": "AIRBORNE",
                    "eventId": "event.valtan.high-jump.airborne.spawn-target-axe",
                    "countPerResolvedTarget": 1,
                    "layout": {"kind": "TARGET_CENTER"},
                    "allowOverlap": False,
                    "maximumTotalObjects": 32,
                },
                {
                    "op": "SET_BOSS_BASE_FIELD",
                    "bossArchetypeId": "BOSS_VALTAN",
                    "field": "maximumHp",
                    "value": 61000,
                },
                {
                    "op": "SET_DAMAGE_RATE",
                    "damageProfileId": "damage.valtan.circular-spin",
                    "value": 325,
                },
            ],
        }

    def test_v1_migration_is_staged_and_lossless(self) -> None:
        source_path = self.root / pipeline.MASTER_REL
        before = pipeline.sha256_file(source_path)
        migrated = self.migrate()
        self.assertEqual(
            1, self.migration_docs[pipeline.MASTER_REL]["formatVersion"]
        )
        self.assertEqual(2, migrated["formatVersion"])
        self.assertEqual(7, len(migrated["patterns"]))
        self.assertEqual(before, pipeline.sha256_file(source_path))
        phase_events = [
            (pattern["patternId"], stage["stageId"], event)
            for pattern in migrated["patterns"]
            for stage in pattern["stages"]
            for event in stage["events"]
            if event["kind"] == "SET_GAMEPLAY_PHASE"
        ]
        self.assertEqual(
            [
                (
                    "VALTAN_ARENA_BREAK_109",
                    "IMPACT",
                    {
                        "eventId": "event.valtan.arena-break-109.impact.set-gameplay-phase-2",
                        "trigger": "ENTER",
                        "kind": "SET_GAMEPLAY_PHASE",
                        "gameplayPhase": 2,
                    },
                )
            ],
            phase_events,
        )
        projected = pipeline.project_v2_products(self.root, self.docs, migrated)
        for relative, text in projected.items():
            self.assertEqual(
                self.docs[relative],
                json.loads(text, object_pairs_hook=pipeline._reject_duplicate_pairs),
                relative,
            )

    def test_split_authoring_round_trips_canonically_and_preserves_products(self) -> None:
        self.assertTrue(self.source_manifest["splitJoinValidated"])
        source_paths = {row["path"] for row in self.source_manifest["files"]}
        self.assertIn(pipeline.GAMEPLAY_AUTHORING_REL, source_paths)
        self.assertIn(pipeline.PRESENTATION_AUTHORING_REL, source_paths)
        self.assertNotIn(pipeline.MASTER_REL, source_paths)
        self.assertEqual(1, self.source_manifest["gameplaySourceVersion"])
        self.assertEqual(1, self.source_manifest["presentationSourceVersion"])
        self.assertEqual(2, self.source_manifest["joinedSourceVersion"])
        migrated = self.migrate()
        gameplay, presentation = pipeline.split_v2_authoring(
            migrated,
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        self.assertEqual(self.docs[pipeline.GAMEPLAY_AUTHORING_REL], gameplay)
        self.assertEqual(self.docs[pipeline.PRESENTATION_AUTHORING_REL], presentation)
        joined = pipeline.join_v2_authoring(
            gameplay,
            presentation,
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        self.assertEqual(
            pipeline.canonical_bytes(migrated), pipeline.canonical_bytes(joined)
        )
        migrated_products = pipeline.project_v2_products(
            self.root, self.docs, migrated
        )
        joined_products = pipeline.project_v2_products(self.root, self.docs, joined)
        self.assertEqual(
            {path: text.encode("utf-8") for path, text in migrated_products.items()},
            {path: text.encode("utf-8") for path, text in joined_products.items()},
        )
        pipeline._preserve_byte_identical_client_products(
            self.root, joined_products
        )
        for relative in (
            pipeline.BINDINGS_REL,
            pipeline.CUES_REL,
            pipeline.COMBAT_PRODUCT_REL,
            pipeline.WORLD_PRODUCT_REL,
        ):
            self.assertEqual(
                (self.root / relative).read_bytes(),
                joined_products[relative].encode("utf-8"),
                relative,
            )
        self.assert_v1_migration_fixture_is_excluded_from_normal_revision_and_load()

    def test_split_authoring_strict_join_rejects_drift_and_role_leaks(self) -> None:
        gameplay = self.docs[pipeline.GAMEPLAY_AUTHORING_REL]
        presentation = self.docs[pipeline.PRESENTATION_AUTHORING_REL]

        cases = []

        wrong_pattern = copy.deepcopy(presentation)
        wrong_pattern["patterns"][0]["patternId"] += ".drift"
        cases.append(("patternId", copy.deepcopy(gameplay), wrong_pattern))

        wrong_stage = copy.deepcopy(presentation)
        wrong_stage["patterns"][0]["stages"][0]["stageId"] += ".drift"
        cases.append(("stageId", copy.deepcopy(gameplay), wrong_stage))

        wrong_action = copy.deepcopy(presentation)
        wrong_action["patterns"][0]["stages"][0]["actionId"] += ".drift"
        cases.append(("actionId", copy.deepcopy(gameplay), wrong_action))

        gameplay_role_leak = copy.deepcopy(gameplay)
        gameplay_role_leak["patterns"][0]["sourceSequenceIndex"] = 1
        cases.append(
            ("gameplay role leak", gameplay_role_leak, copy.deepcopy(presentation))
        )

        presentation_role_leak = copy.deepcopy(presentation)
        presentation_role_leak["patterns"][0]["stages"][0]["durationMs"] = 1
        cases.append(
            (
                "presentation role leak",
                copy.deepcopy(gameplay),
                presentation_role_leak,
            )
        )

        wall_budget = copy.deepcopy(gameplay)
        spin = next(
            stage
            for pattern in wall_budget["patterns"]
            if pattern["patternId"] == "VALTAN_WHIRLWIND"
            for stage in pattern["stages"]
            if stage["stageId"] == "SPIN"
        )
        spin["durationMs"] += 100
        cases.append(("stage wall budget", wall_budget, copy.deepcopy(presentation)))

        cue_occurrence = copy.deepcopy(presentation)
        cue = next(
            cue
            for pattern in cue_occurrence["patterns"]
            for stage in pattern["stages"]
            for cue in stage["effectCues"]
        )
        cue["clipOccurrenceId"] = "missing.clip.occurrence"
        cases.append(
            ("cue occurrence", copy.deepcopy(gameplay), cue_occurrence)
        )

        for label, gameplay_case, presentation_case in cases:
            with self.subTest(label=label), self.assertRaises(pipeline.PipelineError):
                pipeline.join_v2_authoring(
                    gameplay_case,
                    presentation_case,
                    self.docs[pipeline.WORLD_SET_REL],
                    self.docs[pipeline.COMBAT_AUTHORING_REL],
                )

        missing_volley = copy.deepcopy(gameplay)
        airborne = next(
            stage
            for pattern in missing_volley["patterns"]
            if pattern["patternId"] == "VALTAN_HIGH_JUMP"
            for stage in pattern["stages"]
            if stage["stageId"] == "AIRBORNE"
        )
        airborne["events"] = []
        with self.assertRaisesRegex(pipeline.PipelineError, "HIGH_JUMP/AIRBORNE"):
            pipeline.join_v2_authoring(
                missing_volley,
                copy.deepcopy(presentation),
                self.docs[pipeline.WORLD_SET_REL],
                self.docs[pipeline.COMBAT_AUTHORING_REL],
            )

        missing_phase = copy.deepcopy(gameplay)
        impact = next(
            stage
            for pattern in missing_phase["patterns"]
            if pattern["patternId"] == "VALTAN_ARENA_BREAK_109"
            for stage in pattern["stages"]
            if stage["stageId"] == "IMPACT"
        )
        impact["events"] = [
            event
            for event in impact["events"]
            if event["kind"] != "SET_GAMEPLAY_PHASE"
        ]
        with self.assertRaisesRegex(pipeline.PipelineError, "ARENA_BREAK_109/IMPACT"):
            pipeline.join_v2_authoring(
                missing_phase,
                copy.deepcopy(presentation),
                self.docs[pipeline.WORLD_SET_REL],
                self.docs[pipeline.COMBAT_AUTHORING_REL],
            )

    def assert_v1_migration_fixture_is_excluded_from_normal_revision_and_load(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            repository_root = Path(temporary) / "repository"
            for row in self.source_manifest["files"]:
                source = self.root / row["path"]
                destination = repository_root / row["path"]
                destination.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(source, destination)
            fixture = repository_root / pipeline.MASTER_REL
            fixture.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(self.root / pipeline.MASTER_REL, fixture)

            before = pipeline.source_manifest(repository_root)
            fixture.write_bytes(b"not migration JSON\n")
            after = pipeline.source_manifest(repository_root)
            self.assertEqual(before, after)
            normal_docs = pipeline.load_pipeline_documents(repository_root)
            self.assertNotIn(pipeline.MASTER_REL, normal_docs)
            validation = pipeline.validate_repository(repository_root)
            self.assertEqual(1, validation["gameplaySourceVersion"])
            self.assertEqual(1, validation["presentationSourceVersion"])
            self.assertEqual(2, validation["joinedSourceVersion"])

            patch = self.draft_patch()
            patch["sourceRevision"] = before["sourceManifestId"]
            saved = pipeline.save_authoring(
                repository_root,
                Path(temporary) / "authoring",
                patch,
            )
            self.assertRegex(saved["revisionId"], r"^[0-9a-f]{64}$")

            with self.assertRaises(pipeline.PipelineError):
                pipeline.load_pipeline_documents(
                    repository_root,
                    include_split_authoring=False,
                    include_migration_fixture=True,
                )

    def test_world_set_exact_contract_and_pass_through(self) -> None:
        companion = self.docs[pipeline.WORLD_SET_REL]
        self.assertEqual(
            {"schema", "formatVersion", "areaId", "encounterId", "sets"},
            set(companion),
        )
        self.assertEqual(1, len(companion["sets"]))
        event_set = companion["sets"][0]
        self.assertEqual({"worldEventSetId", "members"}, set(event_set))
        self.assertEqual(pipeline.WORLD_SET_ID, event_set["worldEventSetId"])
        self.assertEqual(30, len(event_set["members"]))
        member_fields = {
            "memberId", "bindingId", "groupId", "mutationId", "offsetMs",
            "receiverCollisionId", "enabled",
        }
        self.assertTrue(all(set(member) == member_fields for member in event_set["members"]))
        migrated = self.migrate()
        projected_text = pipeline.project_v2_products(self.root, self.docs, migrated)[
            pipeline.WORLD_PRODUCT_REL
        ]
        source_text = pipeline.read_text(self.root / pipeline.WORLD_PRODUCT_REL)
        source = self.docs[pipeline.WORLD_PRODUCT_REL]
        projected = json.loads(projected_text)
        self.assertEqual((105, 105, 184), (len(projected["groups"]), len(projected["mutations"]), len(projected["bindings"])))
        managed_ids = {member["bindingId"] for member in event_set["members"]}
        self.assertEqual(30, len(managed_ids))
        source_other = [row for row in source["bindings"] if row["bindingId"] not in managed_ids]
        projected_other = [row for row in projected["bindings"] if row["bindingId"] not in managed_ids]
        self.assertEqual(154, len(source_other))
        self.assertEqual(source_other, projected_other)
        self.assertEqual(source["groups"], projected["groups"])
        self.assertEqual(source["mutations"], projected["mutations"])
        self.assertEqual(source["provenance"], projected["provenance"])
        pipeline._assert_unmanaged_raw_rows_preserved(
            source_text, projected_text, "bindings", "bindingId", managed_ids
        )

    def test_combat_companion_has_only_object_owned_fields(self) -> None:
        companion = self.docs[pipeline.COMBAT_AUTHORING_REL]
        pipeline.validate_combat_authoring(companion)
        self.assertEqual(2, len(companion["objects"]))
        axe = next(
            row for row in companion["objects"]
            if row["combatObjectArchetypeId"] == "combatobject.valtan.high-jump.target-axe"
        )
        self.assertEqual({"kind": "RESOLVED_VOLLEY_POSITION"}, axe["spawn"]["origin"])
        forbidden = {"ownerPatternId", "ownerStageActionId", "lifeMs", "clientVisualId"}
        self.assertFalse(forbidden & set(axe))

    def test_legacy_manifest_seals_unmanaged_closure(self) -> None:
        legacy = self.docs[pipeline.LEGACY_REL]
        pipeline.validate_legacy_manifest(legacy, set(pipeline.MANAGED_PATTERN_IDS))
        self.assertEqual(26, len(legacy["patternEntries"]))
        red_blade = next(
            row for row in legacy["legacyCombatObjectOwners"]
            if row["combatObjectArchetypeId"] == "combatobject.valtan.red-blade-wave.projectile"
        )
        self.assertEqual("VALTAN_RED_BLADE_WAVE", red_blade["ownerPatternId"])
        mixed = next(
            row for row in legacy["sharedRotationRows"]
            if row["rotationId"] == "rotation.valtan.100.84"
        )
        self.assertEqual(
            ["LEGACY", "LEGACY", "MANAGED", "LEGACY", "MANAGED"],
            [ref["ownership"] for ref in mixed["patternRefs"]],
        )

    def test_strict_negative_fixtures(self) -> None:
        world_sets = copy.deepcopy(self.docs[pipeline.WORLD_SET_REL])
        world_sets["sets"][0]["members"][0]["patternId"] = pipeline.WORLD_PATTERN_ID
        with self.assertRaises(pipeline.PipelineError):
            pipeline.validate_world_event_sets(
                world_sets, self.docs[pipeline.WORLD_PRODUCT_REL], migration_fixture=True
            )
        world_sets = copy.deepcopy(self.docs[pipeline.WORLD_SET_REL])
        world_sets["sets"][0]["members"][1]["memberId"] = world_sets["sets"][0]["members"][0]["memberId"]
        with self.assertRaises(pipeline.PipelineError):
            pipeline.validate_world_event_sets(
                world_sets, self.docs[pipeline.WORLD_PRODUCT_REL], migration_fixture=True
            )
        world_sets = copy.deepcopy(self.docs[pipeline.WORLD_SET_REL])
        world_sets["sets"][0]["members"][0]["groupId"] = "destroyable.group.invalid"
        with self.assertRaises(pipeline.PipelineError):
            pipeline.validate_world_event_sets(
                world_sets, self.docs[pipeline.WORLD_PRODUCT_REL], migration_fixture=True
            )
        migrated = self.migrate()
        cue = next(
            cue
            for pattern in migrated["patterns"]
            for stage in pattern["stages"]
            for cue in stage["effectCues"]
        )
        cue["stageOffsetMs"] = 10
        with self.assertRaises(pipeline.PipelineError):
            pipeline.validate_v2_master(
                migrated,
                self.docs[pipeline.WORLD_SET_REL],
                self.docs[pipeline.COMBAT_AUTHORING_REL],
            )
        drifted_docs = copy.deepcopy(self.docs)
        legacy_ordinal = self.docs[pipeline.LEGACY_REL]["patternEntries"][0]["encounterOrdinal"]
        drifted_docs[pipeline.ENCOUNTER_REL]["patterns"][legacy_ordinal]["displayName"] += " drift"
        with self.assertRaisesRegex(pipeline.PipelineError, "sealed legacy encounter row drift"):
            pipeline.validate_legacy_products(
                self.docs[pipeline.LEGACY_REL],
                drifted_docs,
                set(pipeline.MANAGED_PATTERN_IDS),
            )

    def test_mapping_basis_vocabulary_rejects_occurrence_and_cue_typos(self) -> None:
        self.assertEqual(
            frozenset(
                {
                    "CURRENT_PRODUCT_BASELINE",
                    "PATTERN_PR_REFERENCE",
                    "ANIMATION_PR_127",
                    "SOURCE_REVIEWED_DELTA",
                    "PROJECT_AUTHORED",
                    "LEGACY_V1_MIGRATION",
                }
            ),
            pipeline.ALLOWED_MAPPING_BASES,
        )
        invalid_occurrence = self.migrate()
        occurrence = next(
            occurrence
            for pattern in invalid_occurrence["patterns"]
            for stage in pattern["stages"]
            for occurrence in stage["animation"]["occurrences"]
        )
        occurrence["mappingBasis"] = "PROJECT_AUTHORDE"
        with self.assertRaisesRegex(
            pipeline.PipelineError,
            "mappingBasis vocabulary",
        ):
            pipeline.validate_v2_master(
                invalid_occurrence,
                self.docs[pipeline.WORLD_SET_REL],
                self.docs[pipeline.COMBAT_AUTHORING_REL],
            )

        invalid_cue = self.migrate()
        cue = next(
            cue
            for pattern in invalid_cue["patterns"]
            for stage in pattern["stages"]
            for cue in stage["effectCues"]
        )
        cue["mappingBasis"] = "PROJECT_AUTHORDE"
        with self.assertRaisesRegex(
            pipeline.PipelineError,
            "mappingBasis vocabulary",
        ):
            pipeline.validate_v2_master(
                invalid_cue,
                self.docs[pipeline.WORLD_SET_REL],
                self.docs[pipeline.COMBAT_AUTHORING_REL],
            )

    def test_source_hash_precondition_and_revision_manifest(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            candidate_root = Path(temporary) / "candidates"
            pointer = pipeline.publish_candidate(
                self.root,
                candidate_root,
                expected_source_manifest=self.source_manifest,
            )
            self.assertFalse(pointer["activeRuntimeChanged"])
            revision = pointer["revisionId"]
            manifest = pipeline.read_json(
                candidate_root / "revisions" / revision / "revision-manifest.json"
            )
            identity_payload = copy.deepcopy(manifest)
            identity_payload["revisionId"] = ""
            self.assertEqual(revision, pipeline.canonical_hash(identity_payload))
            self.assertEqual("PARENT_MANIFEST", manifest["revisionIdentity"]["kind"])
            identity_path = (
                candidate_root
                / "revisions"
                / revision
                / manifest["revisionIdentity"]["identityPayloadPath"]
            )
            self.assertEqual(revision, pipeline.sha256_file(identity_path))
            self.assertEqual(identity_payload, pipeline.read_json(identity_path))
            self.assertNotIn(
                "revision-identity.json", {row["path"] for row in manifest["artifacts"]}
            )
            pipeline.validate_candidate_revision_manifest(
                candidate_root / "revisions" / revision,
                manifest,
            )
            invalid_activation = copy.deepcopy(manifest)
            invalid_activation["runtimeActivation"] = "NOT_IMPLEMENTED"
            with self.assertRaisesRegex(
                pipeline.PipelineError, "activation contract"
            ):
                pipeline.validate_candidate_revision_manifest(
                    candidate_root / "revisions" / revision,
                    invalid_activation,
                )
            self.assertEqual("SERVER_2PC_TICK_BOUNDARY", manifest["runtimeActivation"])
            self.assertEqual("HOT_RELOAD", manifest["applyClass"])
            self.assertEqual(["VALTAN_BOSS"], manifest["allowedDomains"])
            expected_lanes = {
                "ANIMATION", "EFFECT", "COMBAT_VISUAL", "CAMERA", "WORLD_EVENT_SET"
            }
            self.assertEqual(expected_lanes, set(manifest["requiredPresentationLanes"]))
            compatibility = manifest["clientPresentationCompatibility"]
            self.assertEqual("BYTE_IDENTICAL_TO_ACTIVE", compatibility["mode"])
            self.assertEqual(expected_lanes, set(compatibility["requiredLanes"]))
            self.assertEqual(expected_lanes, {row["lane"] for row in compatibility["artifacts"]})
            revision_root = candidate_root / "revisions" / revision
            for artifact in compatibility["artifacts"]:
                staged = revision_root / artifact["path"]
                source = self.root / artifact["path"]
                self.assertEqual(source.read_bytes(), staged.read_bytes())
                self.assertEqual(artifact["sha256"], artifact["repositorySourceSha256"])
                self.assertEqual(artifact["bytes"], staged.stat().st_size)
            bootstrap = revision_root / pipeline.GAMEPLAY_BOOTSTRAP_REL
            self.assertTrue(bootstrap.is_file())
            bootstrap_contract = manifest["serverGameplayBootstrap"]
            self.assertEqual(pipeline.GAMEPLAY_BOOTSTRAP_REL, bootstrap_contract["path"])
            self.assertEqual(
                pipeline.sha256_file(bootstrap), bootstrap_contract["candidateSha256"]
            )
            self.assertEqual(
                bootstrap_contract["candidateSha256"],
                manifest["revisionIdentity"]["serverBootstrapContentRevision"],
            )
            server_submanifest = pipeline.read_json(revision_root / "_manifest/server.json")
            self.assertIn(
                pipeline.GAMEPLAY_BOOTSTRAP_REL,
                {row["path"] for row in server_submanifest["artifacts"]},
            )
            second = pipeline.publish_candidate(
                self.root,
                candidate_root,
                expected_source_manifest=self.source_manifest,
            )
            self.assertEqual(pointer, second)
            self.assertEqual(self.source_manifest, pipeline.source_manifest(self.root))
            wrong = copy.deepcopy(self.source_manifest)
            wrong["sourceManifestId"] = "0" * 64
            with self.assertRaises(pipeline.PipelineError):
                pipeline.publish_candidate(
                    self.root,
                    candidate_root,
                    expected_source_manifest=wrong,
                )

    def test_gameplay_bootstrap_diff_is_valtan_only_fail_closed(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            baseline = root / "baseline.bootstrap"
            candidate = root / "candidate.bootstrap"

            def write_bootstrap(path: Path, rows: list[str]) -> None:
                path.write_text(
                    f"LOSTARK_GAMEPLAY_BOOTSTRAP\t{pipeline.GAMEPLAY_BOOTSTRAP_VERSION}\t"
                    + str(len(rows))
                    + "\n"
                    + "\n".join(rows)
                    + "\n",
                    encoding="utf-8",
                )

            write_bootstrap(
                baseline,
                ["PLAYER\tLANCE_MASTER\t100", "DAMAGE\tdamage.valtan.swing\t220"],
            )
            write_bootstrap(
                candidate,
                ["PLAYER\tLANCE_MASTER\t100", "DAMAGE\tdamage.valtan.swing\t221"],
            )
            result = pipeline.validate_valtan_only_bootstrap_diff(baseline, candidate)
            self.assertEqual((1, 1), (result["removedValtanRows"], result["addedValtanRows"]))

            write_bootstrap(
                candidate,
                ["PLAYER\tLANCE_MASTER\t101", "DAMAGE\tdamage.valtan.swing\t221"],
            )
            with self.assertRaisesRegex(
                pipeline.PipelineError, "outside VALTAN_BOSS"
            ):
                pipeline.validate_valtan_only_bootstrap_diff(baseline, candidate)

    def test_candidate_apply_class_uses_final_saved_boss_state(self) -> None:
        runtime_bosses = self.docs[pipeline.BOSS_PROFILES_REL]
        unchanged_candidate = copy.deepcopy(runtime_bosses)
        self.assertEqual(
            "HOT_RELOAD",
            pipeline.classify_candidate_apply_class(
                runtime_bosses, unchanged_candidate
            ),
        )

        changed_candidate = copy.deepcopy(runtime_bosses)
        runtime_valtan = next(
            row
            for row in runtime_bosses["bosses"]
            if row["archetypeId"] == "BOSS_VALTAN"
        )
        candidate_valtan = next(
            row
            for row in changed_candidate["bosses"]
            if row["archetypeId"] == "BOSS_VALTAN"
        )
        candidate_valtan["maximumHp"] = runtime_valtan["maximumHp"] + 1
        self.assertEqual(
            "ENCOUNTER_RESET",
            pipeline.classify_candidate_apply_class(
                runtime_bosses, changed_candidate
            ),
        )

        patch = {
            "schema": pipeline.DRAFT_PATCH_SCHEMA,
            "formatVersion": 1,
            "sourceRevision": self.source_manifest["sourceManifestId"],
            "operations": [
                {
                    "op": "SET_BOSS_BASE_FIELD",
                    "bossArchetypeId": "BOSS_VALTAN",
                    "field": "maximumHp",
                    "value": runtime_valtan["maximumHp"] + 1,
                }
            ],
        }
        with tempfile.TemporaryDirectory() as temporary:
            temporary_root = Path(temporary)
            authoring_root = temporary_root / "authoring"
            saved = pipeline.save_authoring(self.root, authoring_root, patch)
            candidate_root = temporary_root / "candidates"
            published = pipeline.publish_candidate(
                self.root,
                candidate_root,
                authoring_root=authoring_root,
                authoring_revision=saved["revisionId"],
            )
            revision_root = (
                candidate_root / "revisions" / published["revisionId"]
            )
            manifest = pipeline.read_json(
                revision_root / "revision-manifest.json"
            )
            self.assertEqual(0, manifest["draftPatchOperationCount"])
            self.assertEqual("ENCOUNTER_RESET", manifest["applyClass"])

            invalid = copy.deepcopy(manifest)
            invalid["applyClass"] = "UNRECOGNIZED_APPLY_CLASS"
            with self.assertRaisesRegex(
                pipeline.PipelineError, "activation contract"
            ):
                pipeline.validate_candidate_revision_manifest(
                    revision_root, invalid
                )

    def test_stable_id_draft_patch_projects_balance_and_pattern_candidate(self) -> None:
        draft = self.draft_patch()
        migrated = self.migrate()
        candidate, bosses, damage, count = pipeline.apply_draft_patch(
            migrated,
            self.docs[pipeline.BOSS_PROFILES_REL],
            self.docs[pipeline.DAMAGE_REL],
            draft,
            self.source_manifest["sourceManifestId"],
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        self.assertEqual(9, count)
        whirlwind = next(row for row in candidate["patterns"] if row["patternId"] == "VALTAN_WHIRLWIND")
        self.assertEqual(3, whirlwind["eligibility"]["repeatPolicy"]["limit"])
        self.assertEqual((1.0, 13.0), (whirlwind["eligibility"]["minimumRangeM"], whirlwind["eligibility"]["maximumRangeM"]))
        spin = next(row for row in whirlwind["stages"] if row["stageId"] == "SPIN")
        self.assertEqual([0, 300, 600, 900], spin["hit"]["schedule"]["offsetsMs"])
        self.assertEqual(61000, bosses["bosses"][0]["maximumHp"])
        damage_row = next(
            row for row in damage["profiles"]
            if row["damageProfileId"] == "damage.valtan.circular-spin"
        )
        self.assertEqual(325, damage_row["damageRatePercent"])

        with tempfile.TemporaryDirectory() as temporary:
            candidate_root = Path(temporary) / "candidates"
            pointer = pipeline.publish_candidate(
                self.root,
                candidate_root,
                expected_source_manifest=self.source_manifest,
                draft_patch=draft,
            )
            revision_root = candidate_root / "revisions" / pointer["revisionId"]
            encounter = pipeline.read_json(revision_root / pipeline.ENCOUNTER_REL)
            projected = next(row for row in encounter["patterns"] if row["patternId"] == "VALTAN_WHIRLWIND")
            self.assertEqual(
                20, projected["selectionWeight"],
                "per-set edits must not rewrite the explicit global fallback",
            )
            self.assertEqual(1400, projected["stages"][0]["durationMs"])
            projected_bosses = pipeline.read_json(revision_root / pipeline.BOSS_PROFILES_REL)
            self.assertEqual(61000, projected_bosses["bosses"][0]["maximumHp"])
            projected_damage = pipeline.read_json(revision_root / pipeline.DAMAGE_REL)
            self.assertEqual(
                325,
                next(
                    row["damageRatePercent"]
                    for row in projected_damage["profiles"]
                    if row["damageProfileId"] == "damage.valtan.circular-spin"
                ),
            )
            manifest = pipeline.read_json(revision_root / "revision-manifest.json")
            self.assertEqual(9, manifest["draftPatchOperationCount"])
            self.assertEqual("ENCOUNTER_RESET", manifest["applyClass"])
            self.assertEqual(self.source_manifest["sourceManifestId"], manifest["sourceManifestId"])
            receipt = pipeline.read_json(revision_root / pipeline.PROVENANCE_REL)
            provenance = {
                (row["targetId"], row["targetField"]): row
                for row in receipt["entries"]
            }
            self.assertEqual(
                61000,
                provenance[("boss:BOSS_VALTAN", "maximumHp")]["resultValue"],
            )
            self.assertEqual(
                325,
                provenance[
                    ("damage:damage.valtan.circular-spin", "damageRatePercent")
                ]["resultValue"],
            )
            weight_entry = provenance[
                ("pattern:VALTAN_WHIRLWIND", "patterns[16].selectionWeight")
            ]
            self.assertEqual(20, weight_entry["resultValue"])

    def test_per_set_weight_enabled_and_mechanic_round_trip_to_v3_bootstrap(self) -> None:
        patch = {
            "schema": pipeline.DRAFT_PATCH_SCHEMA,
            "formatVersion": 1,
            "sourceRevision": self.source_manifest["sourceManifestId"],
            "operations": [
                {
                    "op": "SET_PATTERN_WEIGHT",
                    "selectionSetId": "selectionset.valtan.160.130",
                    "patternId": "VALTAN_DASH_CHARGE",
                    "value": 31,
                },
                {
                    "op": "SET_PATTERN_ENABLED",
                    "selectionSetId": "selectionset.valtan.130.109",
                    "patternId": "VALTAN_DASH_CHARGE",
                    "value": False,
                },
                {
                    "op": "SET_MECHANIC_TRIGGER",
                    "mechanicId": "mechanic.valtan-arena-break-109",
                    "patternId": "VALTAN_ARENA_BREAK_109",
                    "healthBar": 109,
                    "triggerOrder": 2,
                },
            ],
        }
        master = pipeline.join_v2_authoring(
            self.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            self.docs[pipeline.PRESENTATION_AUTHORING_REL],
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        staged, _, _, count = pipeline.apply_draft_patch(
            master,
            self.docs[pipeline.BOSS_PROFILES_REL],
            self.docs[pipeline.DAMAGE_REL],
            patch,
            self.source_manifest["sourceManifestId"],
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        self.assertEqual(3, count)
        first, second = staged["decisionModel"]["selectionSets"]
        self.assertEqual(31, first["candidates"][1]["weight"])
        self.assertEqual(30, second["candidates"][1]["weight"])
        self.assertFalse(second["candidates"][1]["enabled"])
        mechanic = staged["decisionModel"]["mechanics"][1]
        self.assertEqual((109, 2), (
            mechanic["trigger"]["healthBar"], mechanic["triggerOrder"]
        ))

        projected = pipeline.project_v2_products(self.root, self.docs, staged)
        rotations = json.loads(projected[pipeline.ROTATIONS_REL])
        self.assertEqual(3, rotations["formatVersion"])
        self.assertEqual(31, rotations["rotations"][0]["candidates"][1]["weight"])
        self.assertEqual(30, rotations["rotations"][1]["candidates"][1]["weight"])
        self.assertFalse(rotations["rotations"][1]["candidates"][1]["enabled"])
        self.assertEqual(
            self.docs[pipeline.ROTATIONS_REL]["rotations"][2:],
            rotations["rotations"][2:],
            "all six post-109 legacy rows must preserve order and duplicates",
        )
        encounter = json.loads(projected[pipeline.ENCOUNTER_REL])
        dash = next(row for row in encounter["patterns"]
                    if row["patternId"] == "VALTAN_DASH_CHARGE")
        arena = next(row for row in encounter["patterns"]
                     if row["patternId"] == "VALTAN_ARENA_BREAK_109")
        self.assertEqual(30, dash["selectionWeight"],
                         "phase-1 set edits must not mutate global fallback")
        self.assertEqual((109, 2), (arena["triggerHealthBar"], arena["triggerOrder"]))

        with tempfile.TemporaryDirectory() as temporary:
            temporary_root = Path(temporary)
            authoring_root = temporary_root / "authoring"
            saved = pipeline.save_authoring(self.root, authoring_root, patch)
            resumed, _, _ = pipeline.load_authoring_revision(
                self.root,
                authoring_root,
                saved["revisionId"],
                self.source_manifest,
                self.docs,
            )
            self.assertEqual(
                staged["decisionModel"], resumed["decisionModel"],
                "a new Tool must resume the exact saved typed decision model",
            )
            candidate_root = temporary_root / "candidates"
            published = pipeline.publish_candidate(
                self.root,
                candidate_root,
                expected_source_manifest=self.source_manifest,
                authoring_root=authoring_root,
                authoring_revision=saved["revisionId"],
            )
            revision_root = candidate_root / "revisions" / published["revisionId"]
            candidate_rotations = pipeline.read_json(
                revision_root / pipeline.ROTATIONS_REL
            )
            self.assertEqual(rotations, candidate_rotations)
            manifest = pipeline.read_json(revision_root / "revision-manifest.json")
            self.assertEqual("HOT_RELOAD", manifest["applyClass"])
            self.assertEqual(19, manifest["serverGameplayBootstrap"]["formatVersion"])
            bootstrap_lines = (
                revision_root / pipeline.GAMEPLAY_BOOTSTRAP_REL
            ).read_text(encoding="utf-8").splitlines()
            self.assertTrue(bootstrap_lines[0].startswith(
                "LOSTARK_GAMEPLAY_BOOTSTRAP\t19\t"
            ))
            self.assertIn(
                "PATTERNROTATIONCANDIDATE\tENCOUNTER_VALTAN\t"
                "rotation.valtan.160.130\t1\tVALTAN_DASH_CHARGE\t31\t1",
                bootstrap_lines,
            )
            self.assertIn(
                "PATTERNROTATIONCANDIDATE\tENCOUNTER_VALTAN\t"
                "rotation.valtan.130.109\t1\tVALTAN_DASH_CHARGE\t30\t0",
                bootstrap_lines,
            )
            self.assertIn(
                "PATTERNROTATIONWINDOW\tENCOUNTER_VALTAN\t"
                "rotation.valtan.160.130\twindow.valtan.phase1.160.130\t1\t"
                "selectionset.valtan.160.130\t160\t130\t5",
                bootstrap_lines,
            )
            self.assertFalse(any(
                line.startswith("PATTERNROTATIONSTEP\tENCOUNTER_VALTAN\t"
                                "rotation.valtan.160.130\t")
                for line in bootstrap_lines
            ))
            self.assertTrue(any(
                line.startswith("PATTERNROTATIONSTEP\tENCOUNTER_VALTAN\t"
                                "rotation.valtan.109.100\t")
                for line in bootstrap_lines
            ))

    def test_decision_draft_and_closure_invalid_inputs_fail_closed(self) -> None:
        base = pipeline.join_v2_authoring(
            self.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            self.docs[pipeline.PRESENTATION_AUTHORING_REL],
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        def apply(operation: dict) -> None:
            staged, _, _, _ = pipeline.apply_draft_patch(
                base,
                self.docs[pipeline.BOSS_PROFILES_REL],
                self.docs[pipeline.DAMAGE_REL],
                {
                    "schema": pipeline.DRAFT_PATCH_SCHEMA,
                    "formatVersion": 1,
                    "sourceRevision": self.source_manifest["sourceManifestId"],
                    "operations": [operation],
                },
                self.source_manifest["sourceManifestId"],
                self.docs[pipeline.WORLD_SET_REL],
                self.docs[pipeline.COMBAT_AUTHORING_REL],
            )
            pipeline.project_v2_products(self.root, self.docs, staged)

        invalid_operations = (
            {
                "op": "SET_PATTERN_WEIGHT", "selectionSetId": "missing.set",
                "patternId": "VALTAN_DASH_CHARGE", "value": 31,
            },
            {
                "op": "SET_PATTERN_WEIGHT",
                "selectionSetId": "selectionset.valtan.160.130",
                "patternId": "VALTAN_DASH_CHARGE", "value": 0,
            },
            {
                "op": "SET_MECHANIC_TRIGGER", "mechanicId": "missing.mechanic",
                "patternId": "VALTAN_ARENA_BREAK_109", "healthBar": 100,
                "triggerOrder": 2,
            },
            {
                "op": "SET_MECHANIC_TRIGGER",
                "mechanicId": "mechanic.valtan-arena-break-109",
                "patternId": "VALTAN_ARENA_BREAK_109", "healthBar": 0,
                "triggerOrder": 1,
            },
            {
                "op": "SET_MECHANIC_TRIGGER",
                "mechanicId": "mechanic.valtan-arena-break-109",
                "patternId": "VALTAN_ARENA_BREAK_109", "healthBar": 161,
                "triggerOrder": 1,
            },
            {
                "op": "SET_MECHANIC_TRIGGER",
                "mechanicId": "mechanic.valtan-arena-break-109",
                "patternId": "VALTAN_ARENA_BREAK_109", "healthBar": 100,
                "triggerOrder": 1,
            },
            {
                "op": "SET_MECHANIC_TRIGGER",
                "mechanicId": "mechanic.valtan-arena-break-109",
                "patternId": "VALTAN_ARENA_BREAK_109", "healthBar": 100,
                "triggerOrder": 2,
            },
        )
        for operation in invalid_operations:
            with self.subTest(operation=operation), self.assertRaises(pipeline.PipelineError):
                apply(operation)

        invalid_masters = []
        duplicate_candidate = copy.deepcopy(base)
        duplicate_candidate["decisionModel"]["selectionSets"][0]["candidates"].append(
            copy.deepcopy(duplicate_candidate["decisionModel"]["selectionSets"][0]["candidates"][0])
        )
        invalid_masters.append(duplicate_candidate)
        zero_weight = copy.deepcopy(base)
        zero_weight["decisionModel"]["selectionSets"][0]["candidates"][0]["weight"] = 0
        invalid_masters.append(zero_weight)
        all_disabled = copy.deepcopy(base)
        for row in all_disabled["decisionModel"]["selectionSets"][0]["candidates"]:
            row["enabled"] = False
        invalid_masters.append(all_disabled)
        overlap = copy.deepcopy(base)
        overlap["decisionModel"]["selectionWindows"][1]["maximumHealthBarInclusive"] = 129
        invalid_masters.append(overlap)
        unknown_set = copy.deepcopy(base)
        unknown_set["decisionModel"]["selectionWindows"][0]["selectionSetId"] = "missing.set"
        invalid_masters.append(unknown_set)
        phase_boundary_drift = copy.deepcopy(base)
        phase_boundary_drift["decisionModel"]["mechanics"][1]["trigger"]["healthBar"] = 100
        invalid_masters.append(phase_boundary_drift)
        for invalid in invalid_masters:
            with self.assertRaises(pipeline.PipelineError):
                pipeline.validate_v2_master(
                    invalid,
                    self.docs[pipeline.WORLD_SET_REL],
                    self.docs[pipeline.COMBAT_AUTHORING_REL],
                )

        coordinated_but_unpromoted = copy.deepcopy(base)
        coordinated_but_unpromoted["decisionModel"]["selectionWindows"][1][
            "minimumHealthBarExclusive"
        ] = 100
        coordinated_but_unpromoted["decisionModel"]["mechanics"][1]["trigger"][
            "healthBar"
        ] = 100
        with self.assertRaisesRegex(
            pipeline.PipelineError, "first legacy rotation boundary"
        ):
            pipeline.project_v2_products(
                self.root, self.docs, coordinated_but_unpromoted
            )

    def test_gameplay_publisher_rejects_partial_phase_boundary_product(self) -> None:
        encounter = copy.deepcopy(self.docs[pipeline.ENCOUNTER_REL])
        arena_break = next(
            row
            for row in encounter["patterns"]
            if row["patternId"] == "VALTAN_ARENA_BREAK_109"
        )
        arena_break["triggerHealthBar"] = 100
        arena_break["triggerOrder"] = 2
        with tempfile.TemporaryDirectory() as temporary:
            overlay_root = Path(temporary)
            encounter_path = overlay_root / pipeline.ENCOUNTER_REL
            encounter_path.parent.mkdir(parents=True)
            encounter_path.write_text(
                json.dumps(encounter, ensure_ascii=False, indent=2) + "\n",
                encoding="utf-8",
            )
            completed = subprocess.run(
                [
                    "powershell", "-NoProfile", "-ExecutionPolicy", "Bypass",
                    "-File",
                    str(
                        self.root
                        / "Tools/GameplayPipeline/Publish-GameplayBalance.ps1"
                    ),
                    "-Mode", "Validate",
                    "-InputOverlayRoot", str(overlay_root),
                ],
                cwd=self.root,
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
        self.assertNotEqual(0, completed.returncode)
        self.assertIn(
            "triggerHealthBar must equal the final phase-1 WINDOW.toHealthBar",
            completed.stdout + completed.stderr,
        )

    def test_draft_patch_revision_duplicate_and_radial_v18_projection(self) -> None:
        wrong = self.draft_patch()
        wrong["sourceRevision"] = "0" * 64
        with self.assertRaisesRegex(pipeline.DraftPatchError, "sourceRevision precondition"):
            pipeline.apply_draft_patch(
                self.migrate(),
                self.docs[pipeline.BOSS_PROFILES_REL],
                self.docs[pipeline.DAMAGE_REL],
                wrong,
                self.source_manifest["sourceManifestId"],
                self.docs[pipeline.WORLD_SET_REL],
                self.docs[pipeline.COMBAT_AUTHORING_REL],
            )

        duplicate = self.draft_patch()
        duplicate["operations"].append(copy.deepcopy(duplicate["operations"][2]))
        with self.assertRaisesRegex(pipeline.DraftPatchError, "duplicate draft target"):
            pipeline.apply_draft_patch(
                self.migrate(),
                self.docs[pipeline.BOSS_PROFILES_REL],
                self.docs[pipeline.DAMAGE_REL],
                duplicate,
                self.source_manifest["sourceManifestId"],
                self.docs[pipeline.WORLD_SET_REL],
                self.docs[pipeline.COMBAT_AUTHORING_REL],
            )

        radial = self.draft_patch()
        volley = next(row for row in radial["operations"] if row["op"] == "SET_AXE_VOLLEY")
        volley["countPerResolvedTarget"] = 3
        volley["layout"] = {
            "kind": "RADIAL_AROUND_TARGET",
            "radiusM": 2.0,
            "startAngleDegrees": 0.0,
            "angleStepDegrees": 120.0,
        }
        staged, _, _, _ = pipeline.apply_draft_patch(
            self.migrate(),
            self.docs[pipeline.BOSS_PROFILES_REL],
            self.docs[pipeline.DAMAGE_REL],
            radial,
            self.source_manifest["sourceManifestId"],
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        projected = pipeline.project_v2_products(self.root, self.docs, staged)
        encounter = json.loads(projected[pipeline.ENCOUNTER_REL])
        high_jump = next(
            row for row in encounter["patterns"]
            if row["patternId"] == "VALTAN_HIGH_JUMP"
        )
        airborne = next(
            row for row in high_jump["stages"] if row["stageId"] == "AIRBORNE"
        )
        self.assertEqual(
            {
                "trigger": "ENTER",
                "kind": "SPAWN_COMBAT_OBJECT_VOLLEY",
                "targetId": "combatobject.valtan.high-jump.target-axe",
                "targetingPolicy": "PER_ALIVE_PLAYER",
                "countPerResolvedTarget": 3,
                "layout": "RADIAL",
                "radiusM": 2.0,
                "startAngleDegrees": 0.0,
                "angleStepDegrees": 120.0,
                "allowOverlap": False,
                "maximumTotalObjects": 32,
            },
            airborne["actions"][0],
        )
        with tempfile.TemporaryDirectory() as temporary:
            candidate_root = Path(temporary) / "candidates"
            pointer = pipeline.publish_candidate(
                self.root,
                candidate_root,
                expected_source_manifest=self.source_manifest,
                draft_patch=radial,
            )
            self.assertFalse(pointer["activeRuntimeChanged"])
            self.assertTrue((candidate_root / "current-candidate.json").exists())

    def test_saved_authoring_revision_chains_and_publishes_without_touching_v1(self) -> None:
        source_path = self.root / pipeline.MASTER_REL
        source_before = pipeline.sha256_file(source_path)
        with tempfile.TemporaryDirectory() as temporary:
            temporary_root = Path(temporary)
            authoring_root = temporary_root / "authoring"
            first = pipeline.save_authoring(self.root, authoring_root, self.draft_patch())
            self.assertFalse(first["activeRuntimeChanged"])
            first_revision = first["revisionId"]
            first_master, first_bosses, first_damage = pipeline.load_authoring_revision(
                self.root,
                authoring_root,
                first_revision,
                self.source_manifest,
                self.docs,
            )
            self.assertEqual(2, first_master["formatVersion"])
            self.assertEqual(61000, first_bosses["bosses"][0]["maximumHp"])
            self.assertEqual(
                325,
                next(
                    row["damageRatePercent"]
                    for row in first_damage["profiles"]
                    if row["damageProfileId"] == "damage.valtan.circular-spin"
                ),
            )

            second_patch = {
                "schema": pipeline.DRAFT_PATCH_SCHEMA,
                "formatVersion": 1,
                "sourceRevision": first_revision,
                "operations": [
                    {
                        "op": "SET_DAMAGE_RATE",
                        "damageProfileId": "damage.valtan.circular-spin",
                        "value": 350,
                    }
                ],
            }
            validation = pipeline.validate_draft_patch(
                self.root, second_patch, authoring_root
            )
            self.assertEqual(first_revision, validation["baseRevision"])
            second = pipeline.save_authoring(self.root, authoring_root, second_patch)
            self.assertNotEqual(first_revision, second["revisionId"])
            candidate_root = temporary_root / "candidates"
            published = pipeline.publish_candidate(
                self.root,
                candidate_root,
                expected_source_manifest=self.source_manifest,
                authoring_root=authoring_root,
                authoring_revision=second["revisionId"],
            )
            revision_root = candidate_root / "revisions" / published["revisionId"]
            bosses = pipeline.read_json(revision_root / pipeline.BOSS_PROFILES_REL)
            damage = pipeline.read_json(revision_root / pipeline.DAMAGE_REL)
            self.assertEqual(61000, bosses["bosses"][0]["maximumHp"])
            self.assertEqual(
                350,
                next(
                    row["damageRatePercent"]
                    for row in damage["profiles"]
                    if row["damageProfileId"] == "damage.valtan.circular-spin"
                ),
            )
            manifest = pipeline.read_json(revision_root / "revision-manifest.json")
            self.assertEqual(second["revisionId"], manifest["authoringBaseRevision"])
        self.assertEqual(source_before, pipeline.sha256_file(source_path))

    def test_authoring_save_uses_current_head_compare_and_swap(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            authoring_root = Path(temporary) / "authoring"
            first = pipeline.save_authoring(
                self.root, authoring_root, self.draft_patch()
            )
            second_patch = {
                "schema": pipeline.DRAFT_PATCH_SCHEMA,
                "formatVersion": 1,
                "sourceRevision": first["revisionId"],
                "operations": [
                    {
                        "op": "SET_DAMAGE_RATE",
                        "damageProfileId": "damage.valtan.circular-spin",
                        "value": 350,
                    }
                ],
            }
            second = pipeline.save_authoring(
                self.root, authoring_root, second_patch
            )
            head_before = (authoring_root / "current-authoring.json").read_bytes()
            revisions_before = {
                row.name for row in (authoring_root / "revisions").iterdir()
            }
            stale_patch = copy.deepcopy(second_patch)
            stale_patch["operations"][0]["value"] = 375
            with self.assertRaisesRegex(
                pipeline.DraftPatchError, "current saved authoring head"
            ) as rejected:
                pipeline.save_authoring(self.root, authoring_root, stale_patch)
            self.assertEqual("SOURCE_REVISION_MISMATCH", rejected.exception.error_code)
            self.assertEqual(
                second["revisionId"],
                pipeline.read_json(authoring_root / "current-authoring.json")[
                    "revisionId"
                ],
            )
            self.assertEqual(
                head_before, (authoring_root / "current-authoring.json").read_bytes()
            )
            self.assertEqual(
                revisions_before,
                {row.name for row in (authoring_root / "revisions").iterdir()},
            )
            self.assertFalse((authoring_root / ".save.lock").exists())
            self.assertFalse((authoring_root / ".save-journal.json").exists())
            with self.assertRaisesRegex(
                pipeline.DraftPatchError, "current saved authoring head"
            ):
                pipeline.validate_draft_patch(
                    self.root, stale_patch, authoring_root
                )
            candidate_root = Path(temporary) / "candidates"
            with self.assertRaisesRegex(
                pipeline.DraftPatchError, "current saved authoring head"
            ):
                pipeline.publish_candidate(
                    self.root,
                    candidate_root,
                    draft_patch=stale_patch,
                    authoring_root=authoring_root,
                )
            with self.assertRaisesRegex(
                pipeline.DraftPatchError, "current saved authoring head"
            ):
                pipeline.publish_candidate(
                    self.root,
                    candidate_root,
                    authoring_root=authoring_root,
                    authoring_revision=first["revisionId"],
                )
            self.assertFalse((candidate_root / "current-candidate.json").exists())
            self.assertFalse((candidate_root / ".publish.lock").exists())

    def test_existing_authoring_revision_collision_revalidates_every_artifact(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            authoring_root = Path(temporary) / "authoring"
            patch = self.draft_patch()
            saved = pipeline.save_authoring(self.root, authoring_root, patch)
            revision_root = authoring_root / "revisions" / saved["revisionId"]
            gameplay_path = revision_root / pipeline.GAMEPLAY_AUTHORING_REL
            original = gameplay_path.read_bytes()
            pipeline._write_fsync(gameplay_path, original + b" ")

            # Re-enter the deterministic same-revision branch without a head.
            # A manifest-only comparison used to accept the tampered immutable
            # directory and recreate a pointer to corrupt bytes.
            (authoring_root / "current-authoring.json").unlink()
            with self.assertRaisesRegex(
                pipeline.PipelineError,
                "authoring artifact hash/path mismatch",
            ):
                pipeline.save_authoring(self.root, authoring_root, patch)

            self.assertFalse((authoring_root / "current-authoring.json").exists())
            self.assertEqual(original + b" ", gameplay_path.read_bytes())
            self.assertFalse((authoring_root / ".save.lock").exists())
            self.assertFalse((authoring_root / ".save-journal.json").exists())

    def test_authoring_save_failure_injection_rolls_back_every_pointer_stage(self) -> None:
        points = (
            "after_stage", "after_validate", "after_revision_manifest",
            "before_promote", "after_promote", "before_pointer", "after_pointer",
        )
        for point in points:
            with self.subTest(point=point), tempfile.TemporaryDirectory() as temporary:
                authoring_root = Path(temporary) / "authoring"
                with self.assertRaises(pipeline.InjectedFailure):
                    pipeline.save_authoring(
                        self.root,
                        authoring_root,
                        self.draft_patch(),
                        fail_at=point,
                    )
                self.assertFalse((authoring_root / "current-authoring.json").exists())
                self.assertFalse((authoring_root / ".save-journal.json").exists())
                self.assertFalse((authoring_root / ".save.lock").exists())
                revisions = authoring_root / "revisions"
                self.assertFalse(revisions.exists() and any(revisions.iterdir()))
                self.assertFalse(any(authoring_root.glob(".stage.*")))
                self.assertFalse(any(authoring_root.glob(".current-authoring.stage.*")))

    def test_authoring_subprocess_hard_crash_recovers_staged_and_promoted(self) -> None:
        tracked = (
            pipeline.MASTER_REL,
            pipeline.COMBAT_AUTHORING_REL,
            pipeline.WORLD_SET_REL,
            pipeline.LEGACY_REL,
            pipeline.BOSS_PROFILES_REL,
            pipeline.DAMAGE_REL,
        )
        before = {relative: pipeline.sha256_file(self.root / relative) for relative in tracked}
        with tempfile.TemporaryDirectory() as temporary:
            temporary_root = Path(temporary)
            authoring_root = temporary_root / "authoring"
            initial = pipeline.save_authoring(
                self.root, authoring_root, self.draft_patch()
            )
            initial_pointer_bytes = (authoring_root / "current-authoring.json").read_bytes()
            next_patch = {
                "schema": pipeline.DRAFT_PATCH_SCHEMA,
                "formatVersion": 1,
                "sourceRevision": initial["revisionId"],
                "operations": [
                    {
                        "op": "SET_DAMAGE_RATE",
                        "damageProfileId": "damage.valtan.circular-spin",
                        "value": 351,
                    }
                ],
            }
            invalid_patch = copy.deepcopy(next_patch)
            invalid_patch["sourceRevision"] = "0" * 64
            next_path = temporary_root / "next.json"
            invalid_path = temporary_root / "invalid.json"
            pipeline._write_fsync(next_path, pipeline.json_text(next_patch).encode("utf-8"))
            pipeline._write_fsync(
                invalid_path, pipeline.json_text(invalid_patch).encode("utf-8")
            )
            base_command = [
                sys.executable,
                str(Path(pipeline.__file__).resolve()),
                "--repository-root",
                str(self.root),
                "save-authoring",
                "--authoring-root",
                str(authoring_root),
            ]

            staged_crash = subprocess.run(
                [
                    *base_command,
                    "--draft-patch",
                    str(next_path),
                    "--crash-at",
                    "after_journal_staged",
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(pipeline.HARD_CRASH_EXIT_CODE, staged_crash.returncode)
            staged_journal = pipeline.read_json(authoring_root / ".save-journal.json")
            staged_target = Path(staged_journal["targetPath"])
            self.assertEqual(initial_pointer_bytes, (authoring_root / "current-authoring.json").read_bytes())
            recovered = subprocess.run(
                [*base_command, "--draft-patch", str(invalid_path)],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(1, recovered.returncode)
            self.assertEqual(initial_pointer_bytes, (authoring_root / "current-authoring.json").read_bytes())
            self.assertFalse(staged_target.exists())
            self.assertFalse((authoring_root / ".save-journal.json").exists())
            self.assertFalse((authoring_root / ".save.lock").exists())
            self.assertFalse(any(authoring_root.glob(".stage.*")))

            promoted_crash = subprocess.run(
                [
                    *base_command,
                    "--draft-patch",
                    str(next_path),
                    "--crash-at",
                    "after_promote",
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(pipeline.HARD_CRASH_EXIT_CODE, promoted_crash.returncode)
            promoted_journal = pipeline.read_json(authoring_root / ".save-journal.json")
            promoted_revision = promoted_journal["revisionId"]
            self.assertEqual(initial_pointer_bytes, (authoring_root / "current-authoring.json").read_bytes())
            recovered = subprocess.run(
                [*base_command, "--draft-patch", str(invalid_path)],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(1, recovered.returncode)
            self.assertEqual(
                promoted_revision,
                pipeline.read_json(authoring_root / "current-authoring.json")["revisionId"],
            )
            self.assertTrue(Path(promoted_journal["targetPath"]).is_dir())
            self.assertFalse((authoring_root / ".save-journal.json").exists())
            self.assertFalse((authoring_root / ".save.lock").exists())

            pointer_patch = {
                "schema": pipeline.DRAFT_PATCH_SCHEMA,
                "formatVersion": 1,
                "sourceRevision": promoted_revision,
                "operations": [
                    {
                        "op": "SET_DAMAGE_RATE",
                        "damageProfileId": "damage.valtan.circular-spin",
                        "value": 352,
                    }
                ],
            }
            pointer_path = temporary_root / "pointer.json"
            pipeline._write_fsync(
                pointer_path, pipeline.json_text(pointer_patch).encode("utf-8")
            )
            pointer_crash = subprocess.run(
                [
                    *base_command,
                    "--draft-patch",
                    str(pointer_path),
                    "--crash-at",
                    "after_pointer",
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(pipeline.HARD_CRASH_EXIT_CODE, pointer_crash.returncode)
            pointer_revision = pipeline.read_json(
                authoring_root / ".save-journal.json"
            )["revisionId"]
            recovered = subprocess.run(
                [*base_command, "--draft-patch", str(invalid_path)],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(1, recovered.returncode)
            self.assertEqual(
                pointer_revision,
                pipeline.read_json(authoring_root / "current-authoring.json")["revisionId"],
            )
            self.assertFalse((authoring_root / ".save-journal.json").exists())
            self.assertFalse((authoring_root / ".save.lock").exists())
        after = {relative: pipeline.sha256_file(self.root / relative) for relative in tracked}
        self.assertEqual(before, after)

    def test_authoring_micro_crash_windows_recover_without_permanent_lockout(self) -> None:
        expectations = {
            "after_staged_journal_temp": "ROLLBACK",
            "after_promoted_journal_temp": "ROLLBACK",
            "after_journal_unlink": "COMMIT",
        }
        for crash_point, outcome in expectations.items():
            with self.subTest(crash_point=crash_point), tempfile.TemporaryDirectory() as temporary:
                temporary_root = Path(temporary)
                authoring_root = temporary_root / "authoring"
                patch_path = temporary_root / "draft.json"
                pipeline._write_fsync(
                    patch_path,
                    pipeline.json_text(self.draft_patch()).encode("utf-8"),
                )
                crashed = subprocess.run(
                    [
                        sys.executable,
                        str(Path(pipeline.__file__).resolve()),
                        "--repository-root",
                        str(self.root),
                        "save-authoring",
                        "--authoring-root",
                        str(authoring_root),
                        "--draft-patch",
                        str(patch_path),
                        "--crash-at",
                        crash_point,
                    ],
                    capture_output=True,
                    text=True,
                    encoding="utf-8",
                    check=False,
                )
                self.assertEqual(
                    pipeline.HARD_CRASH_EXIT_CODE,
                    crashed.returncode,
                    crashed.stderr,
                )
                self.assertTrue((authoring_root / ".save.lock").is_file())
                if crash_point == "after_journal_unlink":
                    committed_revision = pipeline.read_json(
                        authoring_root / "current-authoring.json"
                    )["revisionId"]
                    self.assertFalse(
                        (authoring_root / ".save-journal.json").exists()
                    )
                _, payload, effective_revision = (
                    pipeline.source_manifest_with_authoring(
                        self.root, authoring_root
                    )
                )
                if outcome == "COMMIT":
                    self.assertEqual(committed_revision, effective_revision)
                    self.assertEqual(
                        committed_revision, payload["authoringRevision"]
                    )
                else:
                    self.assertEqual(
                        self.source_manifest["sourceManifestId"], effective_revision
                    )
                    self.assertIsNone(payload["authoringRevision"])
                    self.assertFalse(
                        (authoring_root / "current-authoring.json").exists()
                    )
                    revisions = authoring_root / "revisions"
                    self.assertFalse(
                        revisions.exists() and any(revisions.iterdir())
                    )
                self.assertFalse((authoring_root / ".save.lock").exists())
                self.assertFalse(
                    (authoring_root / ".save-journal.json").exists()
                )
                self.assertFalse(any(authoring_root.glob(".stage.*")))
                self.assertFalse(
                    any(authoring_root.glob(".save-journal.json.stage.*"))
                )

    def test_legacy_mutable_lock_update_and_delete_windows_are_recoverable(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            temporary_root = Path(temporary)
            authoring_root = temporary_root / "authoring"
            patch_path = temporary_root / "draft.json"
            pipeline._write_fsync(
                patch_path,
                pipeline.json_text(self.draft_patch()).encode("utf-8"),
            )
            base_command = [
                sys.executable,
                str(Path(pipeline.__file__).resolve()),
                "--repository-root",
                str(self.root),
                "save-authoring",
                "--authoring-root",
                str(authoring_root),
                "--draft-patch",
                str(patch_path),
            ]
            crashed = subprocess.run(
                [*base_command, "--crash-at", "after_journal_staged"],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(pipeline.HARD_CRASH_EXIT_CODE, crashed.returncode)
            journal_path = authoring_root / ".save-journal.json"
            lock_path = authoring_root / ".save.lock"
            journal = pipeline.read_json(journal_path)
            lock = pipeline.read_json(lock_path)
            lock["journalFileSha256"] = pipeline.sha256_file(journal_path)
            pipeline._write_fsync(
                lock_path, pipeline.json_text(lock).encode("utf-8")
            )
            os.replace(journal["stagePath"], journal["targetPath"])
            journal["state"] = "PROMOTED"
            journal = pipeline._seal_journal(journal)
            pipeline._write_fsync(
                journal_path, pipeline.json_text(journal).encode("utf-8")
            )
            _, payload, committed_revision = pipeline.source_manifest_with_authoring(
                self.root, authoring_root
            )
            self.assertEqual(journal["revisionId"], committed_revision)
            self.assertEqual(committed_revision, payload["authoringRevision"])

            next_patch = {
                "schema": pipeline.DRAFT_PATCH_SCHEMA,
                "formatVersion": 1,
                "sourceRevision": committed_revision,
                "operations": [
                    {
                        "op": "SET_DAMAGE_RATE",
                        "damageProfileId": "damage.valtan.circular-spin",
                        "value": 351,
                    }
                ],
            }
            pipeline._write_fsync(
                patch_path, pipeline.json_text(next_patch).encode("utf-8")
            )
            crashed = subprocess.run(
                [*base_command, "--crash-at", "after_pointer"],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(pipeline.HARD_CRASH_EXIT_CODE, crashed.returncode)
            next_revision = pipeline.read_json(
                authoring_root / "current-authoring.json"
            )["revisionId"]
            lock = pipeline.read_json(lock_path)
            lock["journalFileSha256"] = pipeline.sha256_file(journal_path)
            pipeline._write_fsync(
                lock_path, pipeline.json_text(lock).encode("utf-8")
            )
            journal_path.unlink()
            _, payload, effective_revision = pipeline.source_manifest_with_authoring(
                self.root, authoring_root
            )
            self.assertEqual(next_revision, effective_revision)
            self.assertEqual(next_revision, payload["authoringRevision"])
            self.assertFalse(lock_path.exists())

    def test_cooperating_readers_recover_authoring_hard_crashes_before_read(self) -> None:
        tracked = (
            pipeline.MASTER_REL,
            pipeline.ENCOUNTER_REL,
            pipeline.COMBAT_PRODUCT_REL,
            pipeline.WORLD_PRODUCT_REL,
            pipeline.BOSS_PROFILES_REL,
            pipeline.DAMAGE_REL,
        )
        before = {
            relative: pipeline.sha256_file(self.root / relative)
            for relative in tracked
        }
        with tempfile.TemporaryDirectory() as temporary:
            temporary_root = Path(temporary)
            authoring_root = temporary_root / "authoring"
            candidate_root = temporary_root / "candidates"
            initial = pipeline.save_authoring(
                self.root, authoring_root, self.draft_patch()
            )
            initial_pointer = (authoring_root / "current-authoring.json").read_bytes()
            pipeline_path = str(Path(pipeline.__file__).resolve())
            save_command = [
                sys.executable,
                pipeline_path,
                "--repository-root",
                str(self.root),
                "save-authoring",
                "--authoring-root",
                str(authoring_root),
            ]

            staged_patch = {
                "schema": pipeline.DRAFT_PATCH_SCHEMA,
                "formatVersion": 1,
                "sourceRevision": initial["revisionId"],
                "operations": [
                    {
                        "op": "SET_DAMAGE_RATE",
                        "damageProfileId": "damage.valtan.circular-spin",
                        "value": 351,
                    }
                ],
            }
            staged_path = temporary_root / "staged.json"
            pipeline._write_fsync(
                staged_path, pipeline.json_text(staged_patch).encode("utf-8")
            )
            crashed = subprocess.run(
                [
                    *save_command,
                    "--draft-patch",
                    str(staged_path),
                    "--crash-at",
                    "after_journal_staged",
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(pipeline.HARD_CRASH_EXIT_CODE, crashed.returncode)
            source_reader = subprocess.run(
                [
                    sys.executable,
                    pipeline_path,
                    "--repository-root",
                    str(self.root),
                    "source-manifest",
                    "--authoring-root",
                    str(authoring_root),
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(0, source_reader.returncode, source_reader.stderr)
            self.assertEqual(
                initial_pointer,
                (authoring_root / "current-authoring.json").read_bytes(),
            )
            self.assertFalse((authoring_root / ".save-journal.json").exists())
            self.assertFalse((authoring_root / ".save.lock").exists())
            self.assertFalse(any(authoring_root.glob(".stage.*")))

            crashed = subprocess.run(
                [
                    *save_command,
                    "--draft-patch",
                    str(staged_path),
                    "--crash-at",
                    "after_promote",
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(pipeline.HARD_CRASH_EXIT_CODE, crashed.returncode)
            promoted_revision = pipeline.read_json(
                authoring_root / ".save-journal.json"
            )["revisionId"]
            followup_patch = {
                "schema": pipeline.DRAFT_PATCH_SCHEMA,
                "formatVersion": 1,
                "sourceRevision": promoted_revision,
                "operations": [
                    {
                        "op": "SET_DAMAGE_RATE",
                        "damageProfileId": "damage.valtan.circular-spin",
                        "value": 352,
                    }
                ],
            }
            followup_path = temporary_root / "followup.json"
            pipeline._write_fsync(
                followup_path, pipeline.json_text(followup_patch).encode("utf-8")
            )
            draft_reader = subprocess.run(
                [
                    sys.executable,
                    pipeline_path,
                    "--repository-root",
                    str(self.root),
                    "validate-draft",
                    "--draft-patch",
                    str(followup_path),
                    "--authoring-root",
                    str(authoring_root),
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(0, draft_reader.returncode, draft_reader.stderr)
            self.assertEqual(
                promoted_revision,
                pipeline.read_json(authoring_root / "current-authoring.json")[
                    "revisionId"
                ],
            )

            crashed = subprocess.run(
                [
                    *save_command,
                    "--draft-patch",
                    str(followup_path),
                    "--crash-at",
                    "after_promote",
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(pipeline.HARD_CRASH_EXIT_CODE, crashed.returncode)
            publish_revision = pipeline.read_json(
                authoring_root / ".save-journal.json"
            )["revisionId"]
            candidate_reader = subprocess.run(
                [
                    sys.executable,
                    pipeline_path,
                    "--repository-root",
                    str(self.root),
                    "publish-candidate",
                    "--candidate-root",
                    str(candidate_root),
                    "--authoring-root",
                    str(authoring_root),
                    "--authoring-revision",
                    publish_revision,
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(0, candidate_reader.returncode, candidate_reader.stderr)
            candidate_pointer = pipeline.read_json(
                candidate_root / "current-candidate.json"
            )
            candidate_manifest = pipeline.read_json(
                candidate_root
                / candidate_pointer["manifest"]
            )
            self.assertEqual(
                publish_revision, candidate_manifest["authoringBaseRevision"]
            )
            self.assertEqual(
                publish_revision,
                pipeline.read_json(authoring_root / "current-authoring.json")[
                    "revisionId"
                ],
            )
            self.assertFalse((authoring_root / ".save-journal.json").exists())
            self.assertFalse((authoring_root / ".save.lock").exists())
        after = {
            relative: pipeline.sha256_file(self.root / relative)
            for relative in tracked
        }
        self.assertEqual(before, after)

    def test_source_manifest_exposes_only_a_fully_validated_authoring_base(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            temporary_root = Path(temporary)
            authoring_root = temporary_root / "authoring"
            output_path = temporary_root / "repository-source.json"
            command = [
                sys.executable,
                str(Path(pipeline.__file__).resolve()),
                "--repository-root",
                str(self.root),
                "source-manifest",
                "--authoring-root",
                str(authoring_root),
                "--output",
                str(output_path),
            ]

            no_pointer = subprocess.run(
                command,
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(0, no_pointer.returncode, no_pointer.stderr)
            no_pointer_result = json.loads(no_pointer.stdout)
            self.assertEqual(
                self.source_manifest["sourceManifestId"],
                no_pointer_result["sourceRevision"],
            )
            self.assertIsNone(
                no_pointer_result["payload"]["authoringRevision"]
            )
            self.assertEqual(
                self.source_manifest["sourceManifestId"],
                no_pointer_result["payload"]["sourceManifestId"],
            )
            self.assertEqual(self.source_manifest, pipeline.read_json(output_path))
            self.assertNotIn("authoringRevision", pipeline.read_json(output_path))

            live_root = temporary_root / "live-authoring"
            live_root.mkdir()
            live_lock, _, _ = pipeline._acquire_transaction_lock(
                live_root, "authoring"
            )
            live_command = list(command)
            live_command[live_command.index(str(authoring_root))] = str(live_root)
            try:
                rejected = subprocess.run(
                    live_command,
                    capture_output=True,
                    text=True,
                    encoding="utf-8",
                    check=False,
                )
                self.assertEqual(1, rejected.returncode)
                self.assertIn("live process", rejected.stderr)
                self.assertTrue((live_root / ".save.lock").is_file())
            finally:
                os.close(live_lock)
                (live_root / ".save.lock").unlink(missing_ok=True)

            saved = pipeline.save_authoring(
                self.root, authoring_root, self.draft_patch()
            )
            pointer_path = authoring_root / "current-authoring.json"
            valid_pointer_bytes = pointer_path.read_bytes()
            current = subprocess.run(
                command,
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(0, current.returncode, current.stderr)
            current_result = json.loads(current.stdout)
            self.assertEqual(saved["revisionId"], current_result["sourceRevision"])
            self.assertEqual(
                saved["revisionId"],
                current_result["payload"]["authoringRevision"],
            )
            self.assertEqual(self.source_manifest, pipeline.read_json(output_path))

            corrupt_pointer = pipeline.read_json(pointer_path)
            corrupt_pointer["unexpected"] = True
            pipeline._write_fsync(
                pointer_path,
                pipeline.json_text(corrupt_pointer).encode("utf-8"),
            )
            corrupt_pointer_bytes = pointer_path.read_bytes()
            rejected = subprocess.run(
                command,
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(1, rejected.returncode)
            self.assertEqual(corrupt_pointer_bytes, pointer_path.read_bytes())
            self.assertEqual(self.source_manifest, pipeline.read_json(output_path))

            pipeline._write_fsync(pointer_path, valid_pointer_bytes)
            target = (
                authoring_root
                / "revisions"
                / saved["revisionId"]
                / pipeline.GAMEPLAY_AUTHORING_REL
            )
            target.write_bytes(target.read_bytes() + b" ")
            rejected = subprocess.run(
                command,
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(1, rejected.returncode)
            self.assertEqual(valid_pointer_bytes, pointer_path.read_bytes())
            self.assertTrue(target.is_file())
            self.assertEqual(self.source_manifest, pipeline.read_json(output_path))

    def test_promoted_recovery_rolls_back_when_repository_source_changed(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            temporary_root = Path(temporary)
            repository_root = temporary_root / "repository"
            for row in self.source_manifest["files"]:
                source = self.root / row["path"]
                destination = repository_root / row["path"]
                destination.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(source, destination)
            local_source = pipeline.source_manifest(repository_root)
            patch = self.draft_patch()
            patch["sourceRevision"] = local_source["sourceManifestId"]
            patch_path = temporary_root / "draft.json"
            pipeline._write_fsync(
                patch_path, pipeline.json_text(patch).encode("utf-8")
            )
            authoring_root = temporary_root / "authoring"
            pipeline_path = str(Path(pipeline.__file__).resolve())
            crashed = subprocess.run(
                [
                    sys.executable,
                    pipeline_path,
                    "--repository-root",
                    str(repository_root),
                    "save-authoring",
                    "--authoring-root",
                    str(authoring_root),
                    "--draft-patch",
                    str(patch_path),
                    "--crash-at",
                    "after_promote",
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(pipeline.HARD_CRASH_EXIT_CODE, crashed.returncode)
            journal = pipeline.read_json(authoring_root / ".save-journal.json")
            promoted_target = Path(journal["targetPath"])
            self.assertTrue(promoted_target.is_dir())
            self.assertFalse((authoring_root / "current-authoring.json").exists())

            changed_source_path = repository_root / pipeline.GAMEPLAY_AUTHORING_REL
            changed_source_path.write_bytes(changed_source_path.read_bytes() + b" ")
            changed_source = pipeline.source_manifest(repository_root)
            self.assertNotEqual(
                local_source["sourceManifestId"],
                changed_source["sourceManifestId"],
            )
            recovered = subprocess.run(
                [
                    sys.executable,
                    pipeline_path,
                    "--repository-root",
                    str(repository_root),
                    "source-manifest",
                    "--authoring-root",
                    str(authoring_root),
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(0, recovered.returncode, recovered.stderr)
            result = json.loads(recovered.stdout)
            self.assertEqual(
                changed_source["sourceManifestId"], result["sourceRevision"]
            )
            self.assertIsNone(result["payload"]["authoringRevision"])
            self.assertFalse((authoring_root / "current-authoring.json").exists())
            self.assertFalse(promoted_target.exists())
            self.assertFalse((authoring_root / ".save-journal.json").exists())
            self.assertFalse((authoring_root / ".save.lock").exists())

    def test_candidate_subprocess_hard_crash_recovers_staged_and_promoted(self) -> None:
        tracked = (
            pipeline.MASTER_REL,
            pipeline.ENCOUNTER_REL,
            pipeline.BINDINGS_REL,
            pipeline.CUES_REL,
            pipeline.COMBAT_PRODUCT_REL,
            pipeline.WORLD_PRODUCT_REL,
            pipeline.BOSS_PROFILES_REL,
            pipeline.DAMAGE_REL,
        )
        before = {relative: pipeline.sha256_file(self.root / relative) for relative in tracked}
        with tempfile.TemporaryDirectory() as temporary:
            temporary_root = Path(temporary)
            candidate_root = temporary_root / "candidates"
            initial = pipeline.publish_candidate(
                self.root,
                candidate_root,
                expected_source_manifest=self.source_manifest,
            )
            initial_pointer_bytes = (candidate_root / "current-candidate.json").read_bytes()
            draft_path = temporary_root / "draft.json"
            wrong_source_path = temporary_root / "wrong-source.json"
            pipeline._write_fsync(
                draft_path, pipeline.json_text(self.draft_patch()).encode("utf-8")
            )
            wrong_source = copy.deepcopy(self.source_manifest)
            wrong_source["sourceManifestId"] = "0" * 64
            pipeline._write_fsync(
                wrong_source_path, pipeline.json_text(wrong_source).encode("utf-8")
            )
            base_command = [
                sys.executable,
                str(Path(pipeline.__file__).resolve()),
                "--repository-root",
                str(self.root),
                "publish-candidate",
                "--candidate-root",
                str(candidate_root),
            ]

            staged_crash = subprocess.run(
                [
                    *base_command,
                    "--draft-patch",
                    str(draft_path),
                    "--crash-at",
                    "after_journal_staged",
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(pipeline.HARD_CRASH_EXIT_CODE, staged_crash.returncode)
            staged_journal = pipeline.read_json(candidate_root / ".publish-journal.json")
            staged_target = Path(staged_journal["targetPath"])
            self.assertEqual(initial_pointer_bytes, (candidate_root / "current-candidate.json").read_bytes())
            recovered = subprocess.run(
                [
                    *base_command,
                    "--expected-source-manifest",
                    str(wrong_source_path),
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(1, recovered.returncode)
            self.assertEqual(initial_pointer_bytes, (candidate_root / "current-candidate.json").read_bytes())
            self.assertFalse(staged_target.exists())
            self.assertFalse((candidate_root / ".publish-journal.json").exists())
            self.assertFalse((candidate_root / ".publish.lock").exists())
            self.assertFalse(any(candidate_root.glob(".stage.*")))

            promoted_crash = subprocess.run(
                [
                    *base_command,
                    "--draft-patch",
                    str(draft_path),
                    "--crash-at",
                    "after_promote",
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(pipeline.HARD_CRASH_EXIT_CODE, promoted_crash.returncode)
            promoted_journal = pipeline.read_json(candidate_root / ".publish-journal.json")
            promoted_revision = promoted_journal["revisionId"]
            self.assertEqual(initial_pointer_bytes, (candidate_root / "current-candidate.json").read_bytes())
            recovered = subprocess.run(
                [
                    *base_command,
                    "--expected-source-manifest",
                    str(wrong_source_path),
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(1, recovered.returncode)
            self.assertEqual(
                promoted_revision,
                pipeline.read_json(candidate_root / "current-candidate.json")["revisionId"],
            )
            self.assertTrue(Path(promoted_journal["targetPath"]).is_dir())
            self.assertFalse((candidate_root / ".publish-journal.json").exists())
            self.assertFalse((candidate_root / ".publish.lock").exists())
        after = {relative: pipeline.sha256_file(self.root / relative) for relative in tracked}
        self.assertEqual(before, after)

    def test_candidate_micro_crash_windows_recover_without_permanent_lockout(self) -> None:
        expectations = {
            "after_staged_journal_temp": "ROLLBACK",
            "after_promoted_journal_temp": "ROLLBACK",
            "after_journal_unlink": "COMMIT",
        }
        for crash_point, outcome in expectations.items():
            with self.subTest(crash_point=crash_point), tempfile.TemporaryDirectory() as temporary:
                temporary_root = Path(temporary)
                candidate_root = temporary_root / "candidates"
                patch_path = temporary_root / "draft.json"
                pipeline._write_fsync(
                    patch_path,
                    pipeline.json_text(self.draft_patch()).encode("utf-8"),
                )
                crashed = subprocess.run(
                    [
                        sys.executable,
                        str(Path(pipeline.__file__).resolve()),
                        "--repository-root",
                        str(self.root),
                        "publish-candidate",
                        "--candidate-root",
                        str(candidate_root),
                        "--draft-patch",
                        str(patch_path),
                        "--crash-at",
                        crash_point,
                    ],
                    capture_output=True,
                    text=True,
                    encoding="utf-8",
                    check=False,
                )
                self.assertEqual(
                    pipeline.HARD_CRASH_EXIT_CODE,
                    crashed.returncode,
                    crashed.stderr,
                )
                self.assertTrue((candidate_root / ".publish.lock").is_file())
                if crash_point == "after_journal_unlink":
                    committed_revision = pipeline.read_json(
                        candidate_root / "current-candidate.json"
                    )["revisionId"]
                    self.assertFalse(
                        (candidate_root / ".publish-journal.json").exists()
                    )
                pipeline._recover_durable_transaction(
                    self.root, candidate_root, "candidate"
                )
                if outcome == "COMMIT":
                    self.assertEqual(
                        committed_revision,
                        pipeline.read_json(
                            candidate_root / "current-candidate.json"
                        )["revisionId"],
                    )
                else:
                    self.assertFalse(
                        (candidate_root / "current-candidate.json").exists()
                    )
                    revisions = candidate_root / "revisions"
                    self.assertFalse(
                        revisions.exists() and any(revisions.iterdir())
                    )
                self.assertFalse((candidate_root / ".publish.lock").exists())
                self.assertFalse(
                    (candidate_root / ".publish-journal.json").exists()
                )
                self.assertFalse(any(candidate_root.glob(".stage.*")))
                self.assertFalse(
                    any(candidate_root.glob(".publish-journal.json.stage.*"))
                )

    def test_corrupt_or_escaping_durable_journal_fails_closed_without_cleanup(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            temporary_root = Path(temporary)
            authoring_root = temporary_root / "authoring"
            patch_path = temporary_root / "draft.json"
            pipeline._write_fsync(
                patch_path, pipeline.json_text(self.draft_patch()).encode("utf-8")
            )
            command = [
                sys.executable,
                str(Path(pipeline.__file__).resolve()),
                "--repository-root",
                str(self.root),
                "save-authoring",
                "--authoring-root",
                str(authoring_root),
                "--draft-patch",
                str(patch_path),
                "--crash-at",
                "after_journal_staged",
            ]
            crashed = subprocess.run(
                command,
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(pipeline.HARD_CRASH_EXIT_CODE, crashed.returncode)
            journal_path = authoring_root / ".save-journal.json"
            lock_path = authoring_root / ".save.lock"
            journal = pipeline.read_json(journal_path)
            stage = Path(journal["stagePath"])
            journal["stagePath"] = str(temporary_root / "escape" / stage.name)
            journal = pipeline._seal_journal(journal)
            pipeline._write_fsync(
                journal_path, pipeline.json_text(journal).encode("utf-8")
            )
            with self.assertRaisesRegex(pipeline.PipelineError, "path escapes"):
                pipeline.save_authoring(
                    self.root, authoring_root, self.draft_patch()
                )
            self.assertTrue(lock_path.is_file())
            self.assertTrue(journal_path.is_file())
            self.assertTrue(stage.is_dir())

        with tempfile.TemporaryDirectory() as temporary:
            temporary_root = Path(temporary)
            authoring_root = temporary_root / "authoring"
            patch_path = temporary_root / "draft.json"
            pipeline._write_fsync(
                patch_path, pipeline.json_text(self.draft_patch()).encode("utf-8")
            )
            command = [
                sys.executable,
                str(Path(pipeline.__file__).resolve()),
                "--repository-root",
                str(self.root),
                "save-authoring",
                "--authoring-root",
                str(authoring_root),
                "--draft-patch",
                str(patch_path),
                "--crash-at",
                "after_journal_staged",
            ]
            crashed = subprocess.run(
                command,
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(pipeline.HARD_CRASH_EXIT_CODE, crashed.returncode)
            journal_path = authoring_root / ".save-journal.json"
            lock_path = authoring_root / ".save.lock"
            stage = Path(pipeline.read_json(journal_path)["stagePath"])
            journal_path.write_bytes(journal_path.read_bytes() + b" ")
            with self.assertRaisesRegex(pipeline.PipelineError, "not canonical"):
                pipeline.save_authoring(
                    self.root, authoring_root, self.draft_patch()
                )
            self.assertTrue(lock_path.is_file())
            self.assertTrue(journal_path.is_file())
            self.assertTrue(stage.is_dir())

    def test_revision_parent_reparse_is_rejected_for_authoring_and_candidate(self) -> None:
        for kind in ("authoring", "candidate"):
            with self.subTest(kind=kind), tempfile.TemporaryDirectory() as temporary:
                temporary_root = Path(temporary)
                transaction_root = temporary_root / kind
                outside = temporary_root / (kind + "-outside")
                transaction_root.mkdir()
                outside.mkdir()
                revisions = transaction_root / "revisions"
                self.create_directory_reparse(revisions, outside)
                try:
                    with self.assertRaisesRegex(
                        pipeline.PipelineError, "reparse point"
                    ):
                        if kind == "authoring":
                            pipeline.save_authoring(
                                self.root,
                                transaction_root,
                                self.draft_patch(),
                            )
                        else:
                            pipeline.publish_candidate(
                                self.root,
                                transaction_root,
                                expected_source_manifest=self.source_manifest,
                                draft_patch=self.draft_patch(),
                            )
                    self.assertEqual([], list(outside.iterdir()))
                    pointer_name = (
                        "current-authoring.json"
                        if kind == "authoring"
                        else "current-candidate.json"
                    )
                    self.assertFalse((transaction_root / pointer_name).exists())
                finally:
                    self.remove_directory_reparse(revisions)

    def test_cli_returns_structured_success_and_error_contract(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            temporary_root = Path(temporary)
            patch_path = temporary_root / "draft.json"
            pipeline._write_fsync(patch_path, pipeline.json_text(self.draft_patch()).encode("utf-8"))
            command = [
                sys.executable,
                str(Path(pipeline.__file__).resolve()),
                "--repository-root",
                str(self.root),
                "validate-draft",
                "--draft-patch",
                str(patch_path),
            ]
            completed = subprocess.run(command, capture_output=True, text=True, encoding="utf-8", check=False)
            self.assertEqual(0, completed.returncode, completed.stderr)
            result = json.loads(completed.stdout)
            self.assertEqual(
                {
                    "schema", "formatVersion", "command", "ok", "sourceRevision",
                    "candidateRevision", "errors", "payload",
                },
                set(result),
            )
            self.assertTrue(result["ok"])
            self.assertEqual([], result["errors"])
            self.assertIsNone(result["candidateRevision"])

            bad = self.draft_patch()
            bad["sourceRevision"] = "0" * 64
            pipeline._write_fsync(patch_path, pipeline.json_text(bad).encode("utf-8"))
            completed = subprocess.run(command, capture_output=True, text=True, encoding="utf-8", check=False)
            self.assertEqual(1, completed.returncode)
            result = json.loads(completed.stderr)
            self.assertFalse(result["ok"])
            self.assertEqual("SOURCE_REVISION_MISMATCH", result["errors"][0]["errorCode"])
            self.assertEqual(self.source_manifest["sourceManifestId"], result["sourceRevision"])
            self.assertIsNone(result["candidateRevision"])

            preview_path = temporary_root / "Valtan.pattern.v2.json"
            migrate_command = [
                sys.executable,
                str(Path(pipeline.__file__).resolve()),
                "--repository-root",
                str(self.root),
                "migrate-preview",
                "--output",
                str(preview_path),
            ]
            completed = subprocess.run(
                migrate_command, capture_output=True, text=True, encoding="utf-8", check=False
            )
            self.assertEqual(0, completed.returncode, completed.stderr)
            result = json.loads(completed.stdout)
            self.assertTrue(result["ok"])
            self.assertIsNone(result["candidateRevision"])
            self.assertEqual(2, pipeline.read_json(preview_path)["formatVersion"])

            source_before = pipeline.sha256_file(self.root / pipeline.MASTER_REL)
            migrate_command[-1] = str(self.root / pipeline.MASTER_REL)
            completed = subprocess.run(
                migrate_command, capture_output=True, text=True, encoding="utf-8", check=False
            )
            self.assertEqual(1, completed.returncode)
            result = json.loads(completed.stderr)
            self.assertEqual("SOURCE_OVERWRITE_FORBIDDEN", result["errors"][0]["errorCode"])
            self.assertEqual(source_before, pipeline.sha256_file(self.root / pipeline.MASTER_REL))

            pipeline._write_fsync(
                patch_path, pipeline.json_text(self.draft_patch()).encode("utf-8")
            )
            authoring_root = temporary_root / "authoring"
            save_command = [
                sys.executable,
                str(Path(pipeline.__file__).resolve()),
                "--repository-root",
                str(self.root),
                "save-authoring",
                "--authoring-root",
                str(authoring_root),
                "--draft-patch",
                str(patch_path),
            ]
            completed = subprocess.run(
                save_command, capture_output=True, text=True, encoding="utf-8", check=False
            )
            self.assertEqual(0, completed.returncode, completed.stderr)
            result = json.loads(completed.stdout)
            self.assertTrue(result["ok"])
            self.assertIsNone(result["candidateRevision"])
            authoring_revision = result["payload"]["authoringRevision"]
            self.assertEqual(authoring_revision, result["payload"]["pointer"]["revisionId"])

            candidate_root = temporary_root / "candidates"
            publish_command = [
                sys.executable,
                str(Path(pipeline.__file__).resolve()),
                "--repository-root",
                str(self.root),
                "publish-candidate",
                "--candidate-root",
                str(candidate_root),
                "--authoring-root",
                str(authoring_root),
                "--authoring-revision",
                authoring_revision,
            ]
            completed = subprocess.run(
                publish_command, capture_output=True, text=True, encoding="utf-8", check=False
            )
            self.assertEqual(0, completed.returncode, completed.stderr)
            result = json.loads(completed.stdout)
            self.assertTrue(result["ok"])
            self.assertEqual(authoring_revision, result["sourceRevision"])
            self.assertEqual(authoring_revision, result["payload"]["authoringRevision"])
            self.assertEqual(
                result["candidateRevision"], result["payload"]["pointer"]["revisionId"]
            )
            self.assertEqual(
                "ENCOUNTER_RESET", result["payload"]["applyClass"]
            )

    def test_powershell_balance_tool_entrypoint_saves_and_publishes(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            temporary_root = Path(temporary)
            patch_path = temporary_root / "draft.json"
            pipeline._write_fsync(
                patch_path, pipeline.json_text(self.draft_patch()).encode("utf-8")
            )
            wrapper = self.root / "Tools/ValtanPipeline/Publish-ValtanTuningRuntimeSet.ps1"
            authoring_root = temporary_root / "authoring"
            missing = subprocess.run(
                [
                    "powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
                    str(wrapper), "-Mode", "ValidateDraft", "-RepositoryRoot", str(self.root),
                    "-AuthoringRoot", str(authoring_root),
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(1, missing.returncode)
            self.assertEqual(
                "DRAFT_PATCH_REQUIRED", json.loads(missing.stderr)["errors"][0]["errorCode"]
            )
            completed = subprocess.run(
                [
                    "powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
                    str(wrapper), "-Mode", "SaveAuthoring", "-RepositoryRoot", str(self.root),
                    "-AuthoringRoot", str(authoring_root), "-DraftPatchPath", str(patch_path),
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(0, completed.returncode, completed.stderr)
            saved = json.loads(completed.stdout)
            authoring_revision = saved["payload"]["authoringRevision"]
            self.assertIsNone(saved["candidateRevision"])

            completed = subprocess.run(
                [
                    "powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
                    str(wrapper), "-Mode", "SourceManifest", "-RepositoryRoot", str(self.root),
                    "-AuthoringRoot", str(authoring_root),
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(0, completed.returncode, completed.stderr)
            resumed = json.loads(completed.stdout)
            self.assertEqual(authoring_revision, resumed["sourceRevision"])
            self.assertEqual(
                authoring_revision, resumed["payload"]["authoringRevision"]
            )

            candidate_root = temporary_root / "candidates"
            completed = subprocess.run(
                [
                    "powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
                    str(wrapper), "-Mode", "PublishCandidate", "-RepositoryRoot", str(self.root),
                    "-AuthoringRoot", str(authoring_root), "-AuthoringRevision", authoring_revision,
                    "-CandidateRoot", str(candidate_root),
                ],
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            self.assertEqual(0, completed.returncode, completed.stderr)
            published = json.loads(completed.stdout)
            self.assertEqual(authoring_revision, published["sourceRevision"])
            self.assertEqual(
                published["candidateRevision"],
                published["payload"]["pointer"]["revisionId"],
            )
            self.assertEqual(
                "ENCOUNTER_RESET", published["payload"]["applyClass"]
            )

    def test_failure_injection_preserves_sources_products_and_pointer(self) -> None:
        points = (
            "after_stage", "after_validate", "after_revision_manifest",
            "before_promote", "after_promote", "before_pointer", "after_pointer",
        )
        tracked = (
            pipeline.MASTER_REL,
            pipeline.COMBAT_AUTHORING_REL,
            pipeline.WORLD_SET_REL,
            pipeline.LEGACY_REL,
            pipeline.ENCOUNTER_REL,
            pipeline.BINDINGS_REL,
            pipeline.CUES_REL,
            pipeline.COMBAT_PRODUCT_REL,
            pipeline.WORLD_PRODUCT_REL,
            pipeline.BOSS_PROFILES_REL,
            pipeline.DAMAGE_REL,
        )
        before = {relative: pipeline.sha256_file(self.root / relative) for relative in tracked}
        for point in points:
            with self.subTest(point=point), tempfile.TemporaryDirectory() as temporary:
                candidate_root = Path(temporary) / "candidates"
                with self.assertRaises(pipeline.InjectedFailure):
                    pipeline.publish_candidate(
                        self.root,
                        candidate_root,
                        expected_source_manifest=self.source_manifest,
                        draft_patch=self.draft_patch(),
                        fail_at=point,
                    )
                self.assertFalse((candidate_root / "current-candidate.json").exists())
                self.assertFalse((candidate_root / ".publish-journal.json").exists())
                self.assertFalse((candidate_root / ".publish.lock").exists())
                revisions = candidate_root / "revisions"
                self.assertFalse(revisions.exists() and any(revisions.iterdir()))
                self.assertFalse(any(candidate_root.glob(".stage.*")))
                self.assertFalse(any(candidate_root.glob(".current-candidate.stage.*")))
        after = {relative: pipeline.sha256_file(self.root / relative) for relative in tracked}
        self.assertEqual(before, after)


def main() -> int:
    global REPOSITORY_ROOT
    parser = argparse.ArgumentParser()
    parser.add_argument("--repository-root", type=Path, default=REPOSITORY_ROOT)
    args, unittest_args = parser.parse_known_args()
    REPOSITORY_ROOT = args.repository_root.resolve()
    unittest.main(argv=[sys.argv[0], *unittest_args], verbosity=2, exit=False)
    result = unittest.TestProgram(argv=[sys.argv[0], *unittest_args], testRunner=unittest.TextTestRunner(verbosity=0), exit=False)
    return 0 if result.result.wasSuccessful() else 1


if __name__ == "__main__":
    # Build the suite once so CI receives a conventional non-zero exit code.
    parser = argparse.ArgumentParser()
    parser.add_argument("--repository-root", type=Path, default=REPOSITORY_ROOT)
    args, remaining = parser.parse_known_args()
    REPOSITORY_ROOT = args.repository_root.resolve()
    program = unittest.main(argv=[sys.argv[0], *remaining], verbosity=2, exit=False)
    raise SystemExit(0 if program.result.wasSuccessful() else 1)
