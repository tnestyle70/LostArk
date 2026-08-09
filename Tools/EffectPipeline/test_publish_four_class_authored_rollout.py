#!/usr/bin/env python3

from __future__ import annotations

import json
import os
from pathlib import Path
import tempfile
import unittest
from unittest import mock

import publish_four_class_authored_rollout as publisher


def _stage(
    target: str,
    clip: str,
    *,
    skill_id: int,
    stage_index: int,
    runtime_hash: str = "a" * 64,
    reuse: str | None = None,
) -> dict:
    product_target = reuse or target
    product = {
        "animationAssetId": "Artist",
        "characterClass": "ARTIST",
        "productSkillId": skill_id,
        "stageIndex": stage_index,
        "stageClipIndex": 0,
        "clip": clip,
        "status": "visualBearing",
        "productTargetEffectAssetId": product_target,
        "productAuthoringPath": f"Effects/Authored/{product_target}.effect.json",
        "productDocumentFileSha256": "c" * 64,
        "normalizedRuntimeSha256": runtime_hash,
        "sourceStageTargetEffectAssetId": product_target,
        "documentPath": Path(f"{product_target}.effect.json"),
        "document": {"effectAssetId": product_target},
    }
    row = {
        "animationAssetId": "Artist",
        "characterClass": "ARTIST",
        "productSkillId": skill_id,
        "stageIndex": stage_index,
        "clips": [clip],
        "silent": False,
        "nominalTargetEffectAssetId": target,
        "productTargetEffectAssetId": product_target,
        "normalizedRuntimeSha256": runtime_hash,
        "clipProducts": [product],
    }
    if reuse is not None:
        row["reusesProductTarget"] = reuse
    return row


class FourClassAuthoredRolloutPublisherTests(unittest.TestCase):
    def test_current_source_builds_exact_product_contract_without_writes(self) -> None:
        desired, delete_paths, receipt = publisher._build_desired()

        self.assertEqual(55, len(desired))
        self.assertFalse(delete_paths)
        self.assertIn(publisher.CATALOG_PATH, desired)
        self.assertIn(publisher.ROLLOUT_RECEIPT_PATH, desired)
        self.assertIn(publisher.PROTECTED_MATERIALIZATION_PATH, desired)
        self.assertEqual(
            48,
            sum(
                path.parent == publisher.AUTHORED_ROOT and ".clip" in path.name
                for path in desired
            ),
        )
        self.assertEqual(51, receipt["summary"]["skillCount"])
        self.assertEqual(74, receipt["summary"]["stageCount"])
        self.assertEqual(113, receipt["summary"]["clipOccurrenceCount"])
        self.assertEqual(73, receipt["summary"]["effectBearingStageCount"])
        self.assertEqual(1, receipt["summary"]["sourceIntentionallySilentStageCount"])
        self.assertEqual(0, receipt["summary"]["blockedStageCount"])
        self.assertEqual(102, receipt["summary"]["visualClipOccurrenceCount"])
        self.assertEqual(11, receipt["summary"]["silentClipOccurrenceCount"])
        self.assertEqual(48, receipt["summary"]["derivedClipTargetCount"])
        self.assertEqual(53, receipt["summary"]["retainedStageTargetCount"])
        self.assertEqual(0, receipt["summary"]["trimmedElementCount"])
        self.assertEqual(0, receipt["summary"]["unmappedElementCount"])
        self.assertEqual(101, receipt["summary"]["productTargetCount"])
        self.assertEqual(101, receipt["summary"]["productCueCount"])
        self.assertEqual(101, len(receipt["productTargets"]))
        self.assertEqual(
            48,
            sum(row["derivedClipProjection"] for row in receipt["productTargets"]),
        )
        self.assertEqual(
            "sharedEquivalentProductTarget",
            receipt["repeatedClipEvidence"][0]["resolution"],
        )
        protected_gate = json.loads(
            desired[publisher.PROTECTED_MATERIALIZATION_PATH]
        )["stages"][0]["productGate"]
        dimensionmaster_cue_path = (
            publisher.DATA_ROOT
            / "Animation/Authored/DimensionMaster/DimensionMaster.animevents"
        )
        self.assertEqual(
            publisher._sha256_bytes(desired[publisher.CATALOG_PATH]),
            protected_gate["catalog"]["sha256"],
        )
        self.assertEqual(
            publisher._sha256_bytes(desired[dimensionmaster_cue_path]),
            protected_gate["animationCue"]["sha256"],
        )
        delayed_visual_stage = next(
            row
            for row in receipt["stages"]
            if row["characterClass"] == "LANCE_MASTER"
            and row["productSkillId"] == 34140
            and row["stageIndex"] == 1
        )
        self.assertEqual(
            ["noSelectedCarrier", "visualBearing"],
            [row["status"] for row in delayed_visual_stage["clipProducts"]],
        )
        self.assertEqual(
            "effect.lancemaster.skill.34140.ba2.clip2",
            delayed_visual_stage["clipProducts"][1][
                "productTargetEffectAssetId"
            ],
        )

    def test_duplicate_canonical_product_target_is_rejected(self) -> None:
        first = _stage(
            "effect.artist.skill.31200.authored-baseline",
            "artist_clip_a",
            skill_id=31200,
            stage_index=0,
        )
        second = _stage(
            "effect.artist.skill.31200.authored-baseline",
            "artist_clip_b",
            skill_id=31210,
            stage_index=0,
        )

        with self.assertRaisesRegex(ValueError, "Duplicate canonical product target"):
            publisher._index_product_targets(
                [first["clipProducts"][0], second["clipProducts"][0]]
            )

    def test_source_event_owns_delayed_emitter_tail_without_trim_or_reassignment(
        self,
    ) -> None:
        desired, _delete_paths, receipt = publisher._build_desired()
        stage = next(
            row
            for row in receipt["stages"]
            if row["characterClass"] == "LANCE_MASTER"
            and row["productSkillId"] == 34120
            and row["stageIndex"] == 0
        )
        clip_product = stage["clipProducts"][1]
        element_id = "authored.approx.s006.sprite02"
        self.assertEqual("flm_sk_threetalonstrike_02", clip_product["clip"])
        self.assertIn(element_id, clip_product["elementIds"])
        self.assertNotIn(element_id, stage["clipProducts"][2]["elementIds"])
        ownership = next(
            row
            for row in clip_product["sourceElementOwnership"]
            if row["targetElementId"] == element_id
        )
        self.assertAlmostEqual(2.0314, ownership["sourceTimeSeconds"], places=4)
        self.assertAlmostEqual(
            2.0314, ownership["stageLocalSourceTimeSeconds"], places=4
        )
        derived_path = publisher.AUTHORED_ROOT / (
            "effect.lancemaster.skill.34120.authored-baseline.clip2.effect.json"
        )
        derived = json.loads(desired[derived_path])
        projected = next(row for row in derived["elements"] if row["id"] == element_id)
        projected_delay = projected["detail"]["timing"]["startDelaySeconds"]
        self.assertAlmostEqual(0.6314, projected_delay, places=4)
        self.assertGreater(
            projected_delay, clip_product["playableSourceDurationSeconds"]
        )

    def test_preserved_grouped_material_remains_executable_and_receipt_pinned(
        self,
    ) -> None:
        target = "effect.artist.skill.31000.ba1"
        document = publisher._load_json(
            publisher.AUTHORED_ROOT / f"{target}.effect.json", "test Authored"
        )
        receipt = publisher._load_json(
            publisher.GENERATED_CORRECTION_ROOT
            / "Artist"
            / f"{target}.approximation-receipt.json",
            "test approximation receipt",
        )
        publisher._validate_receipt_material_contract(document, receipt, target)
        preserved = [
            candidate
            for occurrence in receipt["occurrences"]
            for candidate in occurrence["candidates"]
            if candidate.get("selectionDecision") == "selected"
            and candidate.get("materialDecision") == "sourceMaterialPreserved"
        ]
        self.assertEqual(3, len(preserved))
        for candidate in preserved:
            source_profile = candidate["materialProvenance"]["sourceProfile"]
            self.assertIs(source_profile["enabled"], True)
            self.assertEqual(
                "effect.ue3.grouped-translucent.v1",
                source_profile["runtimeShaderProfileId"],
            )

    def test_one_clip_cannot_publish_two_product_cues(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            data_root = Path(directory) / "Data"
            event_path = (
                data_root
                / "Animation/Authored/Artist/Artist.animevents"
            )
            event_path.parent.mkdir(parents=True)
            event_path.write_text(
                'LOSTARK_ANIM_EVENTS 5 "Artist" 0\n', encoding="utf-8"
            )
            first = _stage(
                "effect.artist.skill.31200.authored-baseline",
                "shared_clip",
                skill_id=31200,
                stage_index=0,
            )
            second = _stage(
                "effect.artist.skill.31420.authored-baseline",
                "shared_clip",
                skill_id=31420,
                stage_index=0,
            )
            contract = {
                "animationAssetId": "Artist",
                "classToken": "artist",
            }

            with mock.patch.object(publisher, "DATA_ROOT", data_root):
                with self.assertRaisesRegex(ValueError, "multiple product targets"):
                    publisher._desired_animevent_bytes(
                        contract, [first, second]
                    )

    def test_silent_stage_rejects_stale_authored_output(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            authored_root = Path(directory) / "Authored"
            authored_root.mkdir(parents=True)
            target = "effect.artist.skill.31210.ba2"
            (authored_root / f"{target}.effect.json").write_text(
                "{}\n", encoding="utf-8"
            )
            silent = {
                "animationAssetId": "Artist",
                "characterClass": "ARTIST",
                "productSkillId": 31210,
                "stageIndex": 1,
                "clips": ["silent_clip"],
                "silent": True,
                "nominalTargetEffectAssetId": target,
            }

            with mock.patch.multiple(
                publisher,
                AUTHORED_ROOT=authored_root,
                _load_approximation_receipts=mock.DEFAULT,
            ) as patched:
                patched["_load_approximation_receipts"].return_value = {}
                with self.assertRaisesRegex(
                    ValueError, "Intentionally-silent stage has stale product output"
                ):
                    publisher._attach_documents([silent])

    def test_repeated_clip_rejects_wrong_reuse_target(self) -> None:
        canonical = _stage(
            "effect.artist.skill.31210.ba1",
            "sdm_sk_skykongkong_01",
            skill_id=31210,
            stage_index=0,
        )
        reused = _stage(
            "effect.artist.skill.31210.ba3",
            "sdm_sk_skykongkong_01",
            skill_id=31210,
            stage_index=2,
            reuse="effect.artist.skill.31210.ba4",
        )

        with self.assertRaisesRegex(ValueError, "wrong product target"):
            publisher._resolve_repeated_clips([canonical, reused])

    def test_repeated_clip_rejects_non_equivalent_authored_content(self) -> None:
        first = _stage(
            "effect.artist.skill.31210.ba1",
            "sdm_sk_skykongkong_01",
            skill_id=31210,
            stage_index=0,
            runtime_hash="a" * 64,
        )
        second = _stage(
            "effect.artist.skill.31210.ba3",
            "sdm_sk_skykongkong_01",
            skill_id=31210,
            stage_index=2,
            runtime_hash="b" * 64,
        )

        with self.assertRaisesRegex(ValueError, "P0 repeated clip"):
            publisher._resolve_repeated_clips([first, second])

    def test_transaction_rolls_back_existing_and_new_files_on_replace_failure(
        self,
    ) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            first = root / "first.json"
            second = root / "second.json"
            newly_created = root / "new.json"
            last = root / "last.json"
            stale_projection = root / "stale.json"
            first.write_bytes(b"old-first")
            second.write_bytes(b"old-second")
            last.write_bytes(b"old-last")
            stale_projection.write_bytes(b"old-stale")
            desired = {
                first: b"new-first",
                second: b"new-second",
                newly_created: b"new-created",
                last: b"new-last",
            }
            real_replace = os.replace
            replace_count = 0

            def fail_during_final_commit(source, target):
                nonlocal replace_count
                replace_count += 1
                # Four backups (including the managed deletion), then two
                # commits. Failing the seventh replace proves existing,
                # newly-created, and deleted targets all roll back together.
                if replace_count == 7:
                    raise OSError("injected commit failure")
                return real_replace(source, target)

            with mock.patch.object(
                publisher.os, "replace", side_effect=fail_during_final_commit
            ):
                with self.assertRaisesRegex(OSError, "injected commit failure"):
                    publisher._write_transactionally(
                        desired, {stale_projection}
                    )

            self.assertEqual(b"old-first", first.read_bytes())
            self.assertEqual(b"old-second", second.read_bytes())
            self.assertEqual(b"old-last", last.read_bytes())
            self.assertEqual(b"old-stale", stale_projection.read_bytes())
            self.assertFalse(newly_created.exists())
            self.assertEqual(
                {"first.json", "second.json", "last.json", "stale.json"},
                {path.name for path in root.iterdir()},
            )

    def test_managed_stale_projection_is_deleted_without_temporary_debris(
        self,
    ) -> None:
        with tempfile.TemporaryDirectory() as directory:
            data_root = Path(directory) / "Data"
            authored_root = data_root / "Effects/Authored"
            generated_root = data_root / "Effects/AuthoredCorrections/Generated"
            authored_root.mkdir(parents=True)
            generated_root.mkdir(parents=True)
            stale = authored_root / (
                "effect.artist.skill.31210.ba4.clip2.effect.json"
            )
            desired = authored_root / (
                "effect.artist.skill.31210.ba4.clip1.effect.json"
            )
            stale.write_bytes(b"stale")
            stale_sha = publisher._sha256_file(stale)
            receipt_path = generated_root / "rollout.json"
            receipt_path.write_text(
                "{\n"
                '  "schema": "lostark.four-class-authored-product-rollout",\n'
                '  "version": 2,\n'
                '  "productTargets": [{\n'
                '    "derivedClipProjection": true,\n'
                '    "authoringPath": "Effects/Authored/'
                + stale.name
                + '",\n'
                + f'    "documentFileSha256": "{stale_sha}"\n'
                "  }]\n"
                "}\n",
                encoding="utf-8",
            )
            with mock.patch.multiple(
                publisher,
                DATA_ROOT=data_root,
                AUTHORED_ROOT=authored_root,
                ROLLOUT_RECEIPT_PATH=receipt_path,
            ):
                deletes = publisher._managed_projection_deletes(
                    {desired: b"desired"}
                )
                self.assertEqual({stale.resolve()}, deletes)
                publisher._write_transactionally(
                    {desired: b"desired"}, deletes
                )

            self.assertEqual(b"desired", desired.read_bytes())
            self.assertFalse(stale.exists())
            self.assertEqual(
                {desired.name}, {path.name for path in authored_root.iterdir()}
            )

    def test_managed_projection_manual_edit_requires_explicit_migration(
        self,
    ) -> None:
        with tempfile.TemporaryDirectory() as directory:
            data_root = Path(directory) / "Data"
            authored_root = data_root / "Effects/Authored"
            generated_root = data_root / "Effects/AuthoredCorrections/Generated"
            authored_root.mkdir(parents=True)
            generated_root.mkdir(parents=True)
            projection = authored_root / (
                "effect.artist.skill.31930.authored-baseline.clip1.effect.json"
            )
            seed = b"generated-seed"
            projection.write_bytes(b"f1-authored-product")
            receipt_path = generated_root / "rollout.json"
            receipt_path.write_text(
                json.dumps({
                    "schema": publisher.ROLLOUT_SCHEMA,
                    "version": publisher.ROLLOUT_VERSION,
                    "productTargets": [{
                        "derivedClipProjection": True,
                        "authoringPath": f"Effects/Authored/{projection.name}",
                        "documentFileSha256": publisher._sha256_bytes(seed),
                    }],
                }),
                encoding="utf-8",
            )
            desired = {projection: b"regenerated-projection"}
            with mock.patch.multiple(
                publisher,
                DATA_ROOT=data_root,
                AUTHORED_ROOT=authored_root,
                ROLLOUT_RECEIPT_PATH=receipt_path,
            ):
                with self.assertRaisesRegex(
                    ValueError, "refusing to overwrite.*migrate-managed-projections"
                ):
                    publisher._managed_projection_deletes(desired)
                self.assertFalse(
                    publisher._managed_projection_deletes(
                        desired, allow_managed_drift=True
                    )
                )

            self.assertEqual(b"f1-authored-product", projection.read_bytes())


if __name__ == "__main__":
    unittest.main()
