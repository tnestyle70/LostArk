#!/usr/bin/env python3
from __future__ import annotations

import copy
import json
import shutil
import tempfile
import unittest
from pathlib import Path
from unittest import mock

from Tools.ValtanPipeline import promote_valtan_animation_chains as promoter
from Tools.ValtanPipeline import valtan_tuning_pipeline as pipeline


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
PATTERN_ID = "VALTAN_STAGGER_SLOT"
STAGE_ID = "CHANNEL"
ACTION_ID = "valtan.authoring.stagger-slot.channel"
EVENT_ID = "event.valtan.composition.test-area.volley"
ARCHETYPE_ID = "combatobject.valtan.composition.test-area"
VISUAL_ID = "combatobject.visual.valtan.composition.test-area.v1"
HIT_ID = "hit.valtan.composition.test-area.01"


def spawn_event() -> dict[str, object]:
    return {
        "eventId": EVENT_ID,
        "trigger": "ENTER",
        "kind": "SPAWN_COMBAT_OBJECT_VOLLEY",
        "combatObjectArchetypeId": ARCHETYPE_ID,
        "volleyPolicy": "PER_ALIVE_PLAYER",
        "countPerResolvedTarget": 1,
        "layout": {"kind": "TARGET_CENTER"},
        "spawnSchedule": {
            "kind": "INTERVAL",
            "count": 1,
            "firstOffsetMs": 0,
            "intervalMs": 0,
        },
        "arenaRandom": {"kind": "NONE"},
        "allowOverlap": False,
        "maximumTotalObjects": 4,
    }


def definition(*, at_ms: int = 500, lifetime_ms: int = 2500) -> dict[str, object]:
    return {
        "combatObjectArchetypeId": ARCHETYPE_ID,
        "kind": "FIXED_AREA",
        "lifetimeMs": lifetime_ms,
        "spawn": {
            "origin": {"kind": "RESOLVED_VOLLEY_POSITION"},
            "direction": {"kind": "NONE"},
        },
        "movement": {"kind": "STATIC"},
        "hits": [
            {
                "hitId": HIT_ID,
                "trigger": {"kind": "TIMED", "atMs": at_ms},
                "repeat": {"count": 1, "intervalMs": 0},
                "shape": {"kind": "CIRCLE", "outerRadiusM": 3.0},
                "serverDamageProfileId": "damage.valtan.circular-spin",
                "pushRangeM": 0.0,
                "pushMs": 0,
                "knockdown": False,
                "downMs": 0,
            }
        ],
    }


def visual() -> dict[str, object]:
    return {
        "combatObjectArchetypeId": ARCHETYPE_ID,
        "clientVisualId": VISUAL_ID,
        "effectAssetId": "effect.valtan.red-blade-wave.active",
    }


def add_operation() -> dict[str, object]:
    return {
        "op": "ADD_COMBAT_OBJECT",
        "patternId": PATTERN_ID,
        "stageId": STAGE_ID,
        "actionId": ACTION_ID,
        "spawnEvent": spawn_event(),
        "definition": definition(),
        "visual": visual(),
    }


def update_operation() -> dict[str, object]:
    candidate_definition = definition(at_ms=700, lifetime_ms=3000)
    candidate_definition["hits"][0]["shape"]["outerRadiusM"] = 4.5  # type: ignore[index]
    return {
        "op": "UPDATE_COMBAT_OBJECT",
        "patternId": PATTERN_ID,
        "stageId": STAGE_ID,
        "actionId": ACTION_ID,
        "eventId": EVENT_ID,
        "combatObjectArchetypeId": ARCHETYPE_ID,
        "clientVisualId": VISUAL_ID,
        "spawnEvent": spawn_event(),
        "definition": candidate_definition,
        "visual": visual(),
    }


def remove_operation() -> dict[str, object]:
    return {
        "op": "REMOVE_COMBAT_OBJECT",
        "patternId": PATTERN_ID,
        "stageId": STAGE_ID,
        "actionId": ACTION_ID,
        "eventId": EVENT_ID,
        "combatObjectArchetypeId": ARCHETYPE_ID,
        "clientVisualId": VISUAL_ID,
    }


class ValtanCombatObjectTypedWriterTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.root = REPOSITORY_ROOT
        cls.docs = pipeline.load_pipeline_documents(cls.root)
        cls.master = pipeline.join_v2_authoring(
            cls.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            cls.docs[pipeline.PRESENTATION_AUTHORING_REL],
            cls.docs[pipeline.WORLD_SET_REL],
            cls.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        cls.revision = pipeline.source_manifest(cls.root)["sourceManifestId"]

    def apply(
        self,
        operations: list[dict[str, object]],
        *,
        master: dict[str, object] | None = None,
        combat: dict[str, object] | None = None,
        boss_catalog: dict[str, object] | None = None,
    ) -> tuple[dict[str, object], ...]:
        patch = {
            "schema": pipeline.DRAFT_PATCH_SCHEMA,
            "formatVersion": 1,
            "sourceRevision": self.revision,
            "operations": operations,
        }
        return pipeline.apply_draft_patch(
            copy.deepcopy(self.master if master is None else master),
            copy.deepcopy(self.docs[pipeline.BOSS_PROFILES_REL]),
            copy.deepcopy(self.docs[pipeline.DAMAGE_REL]),
            patch,
            self.revision,
            copy.deepcopy(self.docs[pipeline.WORLD_SET_REL]),
            copy.deepcopy(
                self.docs[pipeline.COMBAT_AUTHORING_REL]
                if combat is None
                else combat
            ),
            repository_root=self.root,
            effect_catalog=copy.deepcopy(self.docs[pipeline.EFFECT_CATALOG_REL]),
            boss_catalog=copy.deepcopy(
                self.docs[pipeline.BOSS_CATALOG_REL]
                if boss_catalog is None
                else boss_catalog
            ),
            include_combat_authoring=True,
            include_boss_catalog=True,
        )

    @staticmethod
    def stage(master: dict[str, object]) -> dict[str, object]:
        pattern = next(
            row for row in master["patterns"] if row["patternId"] == PATTERN_ID  # type: ignore[index]
        )
        return next(row for row in pattern["stages"] if row["stageId"] == STAGE_ID)

    @staticmethod
    def valtan_visuals(boss_catalog: dict[str, object]) -> list[dict[str, object]]:
        boss = next(
            row
            for row in boss_catalog["bosses"]  # type: ignore[index]
            if row["archetypeId"] == "BOSS_VALTAN"
        )
        return boss["combatObjectVisuals"]

    def test_add_update_delete_round_trip_owns_all_three_sources(self) -> None:
        original_master = copy.deepcopy(self.master)
        original_combat = copy.deepcopy(self.docs[pipeline.COMBAT_AUTHORING_REL])
        original_catalog = copy.deepcopy(self.docs[pipeline.BOSS_CATALOG_REL])

        added = self.apply([add_operation()])
        added_master, added_combat, added_catalog = added[0], added[3], added[4]
        self.assertEqual(
            [EVENT_ID],
            [
                row["eventId"]
                for row in self.stage(added_master)["events"]
                if row.get("combatObjectArchetypeId") == ARCHETYPE_ID
            ],
        )
        self.assertEqual(
            [ARCHETYPE_ID],
            [
                row["combatObjectArchetypeId"]
                for row in added_combat["objects"]
                if row["combatObjectArchetypeId"] == ARCHETYPE_ID
            ],
        )
        self.assertEqual(
            [VISUAL_ID],
            [
                row["clientVisualId"]
                for row in self.valtan_visuals(added_catalog)
                if row["combatObjectArchetypeId"] == ARCHETYPE_ID
            ],
        )
        candidate_docs = copy.deepcopy(self.docs)
        candidate_docs[pipeline.COMBAT_AUTHORING_REL] = added_combat
        candidate_docs[pipeline.BOSS_CATALOG_REL] = added_catalog
        projected = pipeline.project_v2_products(
            self.root, candidate_docs, added_master
        )
        product = json.loads(projected[pipeline.COMBAT_PRODUCT_REL])
        self.assertTrue(
            any(
                row["combatObjectArchetypeId"] == ARCHETYPE_ID
                and row["clientVisualId"] == VISUAL_ID
                for row in product["objects"]
            )
        )

        updated = self.apply(
            [update_operation()],
            master=added_master,
            combat=added_combat,
            boss_catalog=added_catalog,
        )
        updated_definition = next(
            row
            for row in updated[3]["objects"]
            if row["combatObjectArchetypeId"] == ARCHETYPE_ID
        )
        self.assertEqual(3000, updated_definition["lifetimeMs"])
        self.assertEqual(4.5, updated_definition["hits"][0]["shape"]["outerRadiusM"])

        removed = self.apply(
            [remove_operation()],
            master=updated[0],
            combat=updated[3],
            boss_catalog=updated[4],
        )
        self.assertEqual(original_master, removed[0])
        self.assertEqual(original_combat, removed[3])
        self.assertEqual(original_catalog, removed[4])

    def assert_add_rejected(
        self, mutate: object, error_code: str = "COMBAT_OBJECT_CONTRACT_INVALID"
    ) -> None:
        operation = add_operation()
        mutate(operation)  # type: ignore[operator]
        with self.assertRaises(pipeline.DraftPatchError) as captured:
            self.apply([operation])
        self.assertEqual(error_code, captured.exception.error_code)

    def test_duplicate_and_dangling_visual_ids_are_rejected(self) -> None:
        existing_event_id = next(
            event["eventId"]
            for _pattern, _stage, event in pipeline._combat_object_spawn_owners(
                self.master
            )
        )
        self.assert_add_rejected(
            lambda operation: operation["spawnEvent"].__setitem__(  # type: ignore[index,union-attr]
                "eventId", existing_event_id
            ),
            "DUPLICATE_STABLE_ID",
        )
        existing_visual = self.valtan_visuals(
            self.docs[pipeline.BOSS_CATALOG_REL]
        )[0]["clientVisualId"]
        self.assert_add_rejected(
            lambda operation: operation["visual"].__setitem__(  # type: ignore[index,union-attr]
                "clientVisualId", existing_visual
            ),
            "DUPLICATE_STABLE_ID",
        )
        self.assert_add_rejected(
            lambda operation: operation["visual"].__setitem__(  # type: ignore[index,union-attr]
                "combatObjectArchetypeId", "combatobject.valtan.dangling"
            ),
            "IDENTITY_MISMATCH",
        )

    def test_lifetime_schedule_and_cardinality_fail_closed(self) -> None:
        self.assert_add_rejected(
            lambda operation: operation["definition"].update(  # type: ignore[index,union-attr]
                definition(at_ms=2500, lifetime_ms=2500)
            )
        )
        self.assert_add_rejected(
            lambda operation: operation["spawnEvent"]["spawnSchedule"].update(  # type: ignore[index]
                {"count": 2, "firstOffsetMs": 11900, "intervalMs": 100}
            )
        )
        self.assert_add_rejected(
            lambda operation: operation["spawnEvent"].update(  # type: ignore[index,union-attr]
                {
                    "countPerResolvedTarget": 2,
                    "layout": {
                        "kind": "RADIAL_AROUND_TARGET",
                        "radiusM": 2.0,
                        "startAngleDegrees": 0.0,
                        "angleStepDegrees": 180.0,
                    },
                    "maximumTotalObjects": 1,
                }
            )
        )

    def test_boss_relative_and_specialized_routes_fail_closed(self) -> None:
        self.assert_add_rejected(
            lambda operation: operation["spawnEvent"].update(  # type: ignore[index,union-attr]
                {"volleyPolicy": "BOSS_RELATIVE"}
            )
        )
        high_jump = next(
            (pattern, stage, event)
            for pattern, stage, event in pipeline._combat_object_spawn_owners(
                self.master
            )
            if event["combatObjectArchetypeId"]
            == pipeline.HIGH_JUMP_AXE_ARCHETYPE_ID
        )
        high_jump_visual = next(
            row
            for row in self.valtan_visuals(self.docs[pipeline.BOSS_CATALOG_REL])
            if row["combatObjectArchetypeId"]
            == pipeline.HIGH_JUMP_AXE_ARCHETYPE_ID
        )
        high_jump_definition = next(
            row
            for row in self.docs[pipeline.COMBAT_AUTHORING_REL]["objects"]
            if row["combatObjectArchetypeId"]
            == pipeline.HIGH_JUMP_AXE_ARCHETYPE_ID
        )
        edited_event = copy.deepcopy(high_jump[2])
        edited_event["maximumTotalObjects"] += 1
        operation = {
            "op": "UPDATE_COMBAT_OBJECT",
            "patternId": high_jump[0]["patternId"],
            "stageId": high_jump[1]["stageId"],
            "actionId": high_jump[1]["actionId"],
            "eventId": high_jump[2]["eventId"],
            "combatObjectArchetypeId": pipeline.HIGH_JUMP_AXE_ARCHETYPE_ID,
            "clientVisualId": high_jump_visual["clientVisualId"],
            "spawnEvent": edited_event,
            "definition": copy.deepcopy(high_jump_definition),
            "visual": copy.deepcopy(high_jump_visual),
        }
        with self.assertRaises(pipeline.DraftPatchError) as captured:
            self.apply([operation])
        self.assertEqual(
            "SPECIALIZED_OPERATION_REQUIRED", captured.exception.error_code
        )

        donut_owner = next(
            (pattern, stage, event)
            for pattern, stage, event in pipeline._combat_object_spawn_owners(
                self.master
            )
            if event["combatObjectArchetypeId"]
            == "combatobject.valtan.fist-in-out.donut"
        )
        donut_definition = next(
            row
            for row in self.docs[pipeline.COMBAT_AUTHORING_REL]["objects"]
            if row["combatObjectArchetypeId"]
            == "combatobject.valtan.fist-in-out.donut"
        )
        donut_visual = next(
            row
            for row in self.valtan_visuals(self.docs[pipeline.BOSS_CATALOG_REL])
            if row["combatObjectArchetypeId"]
            == "combatobject.valtan.fist-in-out.donut"
        )
        edited_definition = copy.deepcopy(donut_definition)
        edited_definition["hits"][0]["shape"]["outerRadiusM"] += 1.0
        update_donut = {
            "op": "UPDATE_COMBAT_OBJECT",
            "patternId": donut_owner[0]["patternId"],
            "stageId": donut_owner[1]["stageId"],
            "actionId": donut_owner[1]["actionId"],
            "eventId": donut_owner[2]["eventId"],
            "combatObjectArchetypeId": donut_definition[
                "combatObjectArchetypeId"
            ],
            "clientVisualId": donut_visual["clientVisualId"],
            "spawnEvent": copy.deepcopy(donut_owner[2]),
            "definition": edited_definition,
            "visual": copy.deepcopy(donut_visual),
        }
        with self.assertRaises(pipeline.DraftPatchError) as captured:
            self.apply([update_donut])
        self.assertEqual(
            "SPECIALIZED_OPERATION_REQUIRED", captured.exception.error_code
        )

    def test_delete_rejects_independent_and_sound_references(self) -> None:
        added = self.apply([add_operation()])
        referenced_master = copy.deepcopy(added[0])
        referenced_master["independentEffects"].append(
            {
                "independentEffectId": "valtan.independent-effect.test-area",
                "displayName": "test",
                "source": {
                    "kind": "SERVER_COMBAT_OBJECT",
                    "spawnEventId": EVENT_ID,
                },
            }
        )
        with self.assertRaises(pipeline.DraftPatchError) as captured:
            self.apply(
                [remove_operation()],
                master=referenced_master,
                combat=added[3],
                boss_catalog=added[4],
            )
        self.assertEqual("DANGLING_REFERENCE", captured.exception.error_code)

        donut_owner = next(
            (pattern, stage, event)
            for pattern, stage, event in pipeline._combat_object_spawn_owners(
                self.master
            )
            if event["combatObjectArchetypeId"]
            == "combatobject.valtan.fist-in-out.donut"
        )
        donut_visual = next(
            row
            for row in self.valtan_visuals(self.docs[pipeline.BOSS_CATALOG_REL])
            if row["combatObjectArchetypeId"]
            == "combatobject.valtan.fist-in-out.donut"
        )
        remove_donut = {
            "op": "REMOVE_COMBAT_OBJECT",
            "patternId": donut_owner[0]["patternId"],
            "stageId": donut_owner[1]["stageId"],
            "actionId": donut_owner[1]["actionId"],
            "eventId": donut_owner[2]["eventId"],
            "combatObjectArchetypeId": "combatobject.valtan.fist-in-out.donut",
            "clientVisualId": donut_visual["clientVisualId"],
        }
        with self.assertRaises(pipeline.DraftPatchError) as captured:
            self.apply([remove_donut])
        self.assertEqual("DANGLING_REFERENCE", captured.exception.error_code)


class ValtanCombatObjectCanonicalCommitTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory(
            prefix="valtan-combat-object-writer-"
        )
        self.root = Path(self.temporary.name).resolve()
        shutil.copytree(REPOSITORY_ROOT / "Data", self.root / "Data")
        self.authoring_root = self.root / "Intermediate/ValtanTuningAuthoring"
        self.revision = pipeline.source_manifest(self.root)["sourceManifestId"]
        self.patch_path = self.root / "combat-object.patch.json"
        self.patch_path.write_text(
            pipeline.json_text(
                {
                    "schema": pipeline.DRAFT_PATCH_SCHEMA,
                    "formatVersion": 1,
                    "sourceRevision": self.revision,
                    "operations": [add_operation()],
                }
            ),
            encoding="utf-8",
        )

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def owner_bytes(self) -> dict[str, bytes]:
        relatives = (
            pipeline.GAMEPLAY_AUTHORING_REL,
            pipeline.COMBAT_AUTHORING_REL,
            pipeline.BOSS_CATALOG_REL,
            pipeline.COMBAT_PRODUCT_REL,
        )
        return {relative: (self.root / relative).read_bytes() for relative in relatives}

    def commit(self, **kwargs: object) -> dict[str, object]:
        root_motion = (
            self.root / promoter.ROOT_MOTION_REL
        ).read_text(encoding="utf-8")
        with mock.patch.object(promoter, "_load_v2_pipeline", return_value=pipeline), mock.patch.object(
            pipeline, "validate_valtan_native_animation_source", return_value={}
        ), mock.patch.object(
            promoter, "_project_candidate_root_motion", return_value=root_motion
        ), mock.patch.object(
            promoter,
            "_validate_effect_v2_bindings_against_candidate_products",
            return_value=None,
        ):
            return promoter.commit_typed_authoring_patch(
                self.root,
                self.patch_path,
                authoring_root=self.authoring_root,
                **kwargs,
            )

    def test_three_owner_commit_and_intermediate_failure_rollback(self) -> None:
        baseline = self.owner_bytes()
        with self.assertRaises(promoter.PromotionError):
            self.commit(inject_failure_after=1)
        self.assertEqual(baseline, self.owner_bytes())

        result = self.commit()
        self.assertEqual(1, result["operationCount"])
        gameplay = pipeline.read_json(self.root / pipeline.GAMEPLAY_AUTHORING_REL)
        combat = pipeline.read_json(self.root / pipeline.COMBAT_AUTHORING_REL)
        catalog = pipeline.read_json(self.root / pipeline.BOSS_CATALOG_REL)
        self.assertTrue(
            any(
                event.get("eventId") == EVENT_ID
                for pattern in gameplay["patterns"]
                for stage in pattern["stages"]
                for event in stage["events"]
            )
        )
        self.assertTrue(
            any(row["combatObjectArchetypeId"] == ARCHETYPE_ID for row in combat["objects"])
        )
        self.assertTrue(
            any(
                visual_row["clientVisualId"] == VISUAL_ID
                for boss in catalog["bosses"]
                if boss["archetypeId"] == "BOSS_VALTAN"
                for visual_row in boss["combatObjectVisuals"]
            )
        )

    def test_boss_catalog_is_in_source_read_set(self) -> None:
        catalog_path = self.root / pipeline.BOSS_CATALOG_REL
        before_revision = pipeline.source_manifest(self.root)
        self.assertIn(
            pipeline.BOSS_CATALOG_REL,
            {row["path"] for row in before_revision["files"]},
        )
        catalog = pipeline.read_json(catalog_path)
        catalog["bosses"][0]["presentationScale"] = 1.01
        catalog_path.write_text(pipeline.json_text(catalog), encoding="utf-8")
        after_revision = pipeline.source_manifest(self.root)
        self.assertNotEqual(
            before_revision["sourceManifestId"],
            after_revision["sourceManifestId"],
        )
        baseline = self.owner_bytes()
        with self.assertRaises(promoter.PromotionError) as captured:
            self.commit()
        self.assertIn("source revision", str(captured.exception))
        self.assertEqual(baseline, self.owner_bytes())

    def test_saved_authoring_revision_carries_boss_catalog_owner(self) -> None:
        patch = pipeline.read_json(self.patch_path)
        with mock.patch.object(
            pipeline, "validate_valtan_native_animation_source", return_value={}
        ):
            pointer = pipeline.save_authoring(
                self.root, self.authoring_root, patch
            )
        revision_root = (
            self.authoring_root / "revisions" / pointer["revisionId"]
        )
        manifest = pipeline.read_json(revision_root / "authoring-manifest.json")
        self.assertEqual(
            pipeline.AUTHORING_MANIFEST_FORMAT_VERSION,
            manifest["formatVersion"],
        )
        self.assertIn(
            pipeline.BOSS_CATALOG_REL,
            {row["path"] for row in manifest["artifacts"]},
        )
        pipeline.validate_combat_object_visual_closure(
            pipeline.read_json(revision_root / pipeline.BOSS_CATALOG_REL),
            pipeline.read_json(revision_root / pipeline.COMBAT_AUTHORING_REL),
            pipeline.read_json(self.root / pipeline.EFFECT_CATALOG_REL),
            self.root,
        )

        self.patch_path.write_text(
            pipeline.json_text(
                {
                    "schema": pipeline.DRAFT_PATCH_SCHEMA,
                    "formatVersion": 1,
                    "sourceRevision": pointer["revisionId"],
                    "operations": [],
                }
            ),
            encoding="utf-8",
        )
        result = self.commit()
        self.assertEqual(0, result["operationCount"])
        self.assertNotEqual(self.revision, result["sourceRevision"])
        self.assertTrue(
            any(
                row["combatObjectArchetypeId"] == ARCHETYPE_ID
                for row in pipeline.read_json(
                    self.root / pipeline.COMBAT_AUTHORING_REL
                )["objects"]
            )
        )

    def test_v1_overlay_cannot_hide_combat_visual_closure_drift(self) -> None:
        patch = pipeline.read_json(self.patch_path)
        with mock.patch.object(
            pipeline, "validate_valtan_native_animation_source", return_value={}
        ):
            pointer = pipeline.save_authoring(
                self.root, self.authoring_root, patch
            )
        revision_root = (
            self.authoring_root / "revisions" / pointer["revisionId"]
        )
        manifest = pipeline.read_json(revision_root / "authoring-manifest.json")
        legacy_artifacts = [
            row
            for row in manifest["artifacts"]
            if row["path"] != pipeline.BOSS_CATALOG_REL
        ]
        legacy_artifact_set = pipeline._manifest_hash(legacy_artifacts)
        legacy_revision = pipeline.sha256_bytes(
            (
                manifest["repositorySourceRevision"]
                + "\n"
                + manifest["baseRevision"]
                + "\n"
                + legacy_artifact_set
                + "\n"
            ).encode("utf-8")
        )
        legacy_root = self.authoring_root / "revisions" / legacy_revision
        shutil.copytree(revision_root, legacy_root)
        (legacy_root / pipeline.BOSS_CATALOG_REL).unlink()
        legacy_manifest = copy.deepcopy(manifest)
        legacy_manifest.update(
            {
                "formatVersion": 1,
                "revisionId": legacy_revision,
                "artifactSetId": legacy_artifact_set,
                "artifacts": legacy_artifacts,
            }
        )
        (legacy_root / "authoring-manifest.json").write_text(
            pipeline.json_text(legacy_manifest), encoding="utf-8"
        )
        (self.authoring_root / "current-authoring.json").write_text(
            pipeline.json_text(
                {
                    "schema": "lostark.valtan-tuning-authoring-pointer",
                    "formatVersion": 1,
                    "revisionId": legacy_revision,
                    "manifest": (
                        f"revisions/{legacy_revision}/authoring-manifest.json"
                    ),
                    "activeRuntimeChanged": False,
                }
            ),
            encoding="utf-8",
        )

        with mock.patch.object(
            pipeline, "validate_valtan_native_animation_source", return_value={}
        ), self.assertRaisesRegex(
            pipeline.PipelineError, "identity sets differ"
        ):
            pipeline.source_manifest_with_authoring(
                self.root, self.authoring_root
            )


if __name__ == "__main__":
    unittest.main()
