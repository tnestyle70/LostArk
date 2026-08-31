#!/usr/bin/env python3

from __future__ import annotations

import copy
import inspect
import json
import math
import re
import struct
import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from Tools.ValtanPipeline.valtan_native_animation_inventory import (  # noqa: E402
    NativeAnimationInventoryError,
    build_valtan_composite_animation_inventory,
    load_valtan_composite_animation_inventory,
    validate_native_source_window,
    validate_valtan_presentation_native_windows,
)
from Tools.ValtanPipeline import promote_valtan_animation_chains as promotion  # noqa: E402
from Tools.ValtanPipeline import valtan_tuning_pipeline as pipeline  # noqa: E402


FILE_HEADER = struct.Struct("<4sHHII")
MODEL_HEADER = struct.Struct("<4sIII4I")
SECTION_DESC = struct.Struct("<IIQQ40s")


def build_wmodel(clips: list[tuple[str, float, float]]) -> bytes:
    payloads = []
    for _name, duration_ticks, ticks_per_second in clips:
        animation = (
            b"WANM"
            + struct.pack("<Iff", 0, duration_ticks, ticks_per_second)
            + b"\0" * 16
        )
        payloads.append(
            FILE_HEADER.pack(b"WINT", 1, 0, 0, len(animation)) + animation
        )
    section_count = len(clips) + 1
    offset = MODEL_HEADER.size + SECTION_DESC.size * section_count
    table = SECTION_DESC.pack(1, 0, offset, 0, b"mesh")
    blobs = b""
    for index, ((name, _duration, _rate), payload) in enumerate(
        zip(clips, payloads, strict=True)
    ):
        table += SECTION_DESC.pack(
            4, index, offset, len(payload), name.encode("ascii")[:40]
        )
        blobs += payload
        offset += len(payload)
    content = (
        MODEL_HEADER.pack(
            b"WMOD", section_count, len(clips), 0, 0, 0, 0, 0
        )
        + table
        + blobs
    )
    return FILE_HEADER.pack(b"WINT", 1, 0, 0, len(content)) + content


def fixture_catalog(
    body: str = "Character/Valtan/body.wmodel",
    animation_set: str = "Character/Valtan/animset.wmodel",
) -> dict:
    return {
        "schema": "lostark.boss-catalog",
        "formatVersion": 5,
        "bosses": [
            {
                "archetypeId": "BOSS_VALTAN",
                "bodyModel": body,
                "animationSetId": animation_set,
            }
        ],
    }


class SyntheticValtanNativeAnimationInventoryTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.resource_root = Path(self.temporary.name) / "Resources"
        self.body = self.resource_root / "Character/Valtan/body.wmodel"
        self.animation_set = (
            self.resource_root / "Character/Valtan/animset.wmodel"
        )
        self.body.parent.mkdir(parents=True)
        self.body.write_bytes(build_wmodel([("idle", 70.0, 30.0)]))
        self.animation_set.write_bytes(
            build_wmodel([("attack", 110.0, 30.0)])
        )

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def inventory(self):
        return build_valtan_composite_animation_inventory(
            fixture_catalog(), self.resource_root
        )

    def test_builds_body_then_animation_set_disjoint_inventory(self) -> None:
        inventory = self.inventory()
        self.assertEqual([source.clip_count for source in inventory.sources], [1, 1])
        self.assertEqual(list(inventory.clips), ["idle", "attack"])
        self.assertEqual(inventory.clips["idle"].rounded_native_duration_ms, 2333)
        self.assertEqual(inventory.clips["attack"].rounded_native_duration_ms, 3667)
        self.assertEqual(len(inventory.sources[0].sha256), 64)

    def test_rejects_duplicate_clip_across_atomic_attach(self) -> None:
        self.animation_set.write_bytes(
            build_wmodel([("idle", 110.0, 30.0)])
        )
        with self.assertRaisesRegex(
            NativeAnimationInventoryError,
            "CModel::Attach_AnimationSet duplicate clip",
        ):
            self.inventory()

    def test_rejects_duplicate_clip_inside_one_wmodel(self) -> None:
        self.body.write_bytes(
            build_wmodel([("idle", 70.0, 30.0), ("idle", 20.0, 30.0)])
        )
        with self.assertRaisesRegex(
            NativeAnimationInventoryError,
            "CModel::Attach_AnimationSet duplicate clip",
        ):
            self.inventory()

    def test_rejects_malformed_wmodel(self) -> None:
        self.animation_set.write_bytes(b"not-a-wmodel")
        with self.assertRaisesRegex(
            NativeAnimationInventoryError,
            "cannot parse native animation inventory",
        ):
            self.inventory()

    def test_rejects_missing_wmodel_without_fallback(self) -> None:
        self.animation_set.unlink()
        with self.assertRaisesRegex(
            NativeAnimationInventoryError,
            "required Valtan WModel is unavailable",
        ):
            self.inventory()

    def test_rejects_unsafe_catalog_asset_path(self) -> None:
        with self.assertRaisesRegex(
            NativeAnimationInventoryError,
            "model asset ID is unsafe",
        ):
            build_valtan_composite_animation_inventory(
                fixture_catalog(body="../body.wmodel"), self.resource_root
            )

    def test_accepts_existing_positive_half_away_native_rounding(self) -> None:
        inventory = self.inventory()
        window = validate_native_source_window(
            inventory,
            clip_name="attack",
            source_start_ms=0,
            play_ms=3667,
            play_rate=1.0,
        )
        self.assertAlmostEqual(window.clip.native_duration_ms, 3666.666666, places=5)
        self.assertEqual(window.rounded_remaining_source_ms, 3667)

    def test_rejects_trim_past_rounded_native_remainder(self) -> None:
        inventory = self.inventory()
        with self.assertRaisesRegex(
            NativeAnimationInventoryError,
            "playMs exceeds native clip remainder",
        ):
            validate_native_source_window(
                inventory,
                clip_name="attack",
                source_start_ms=1000,
                play_ms=2668,
                play_rate=1.0,
            )

    def test_rejects_start_at_or_past_native_end(self) -> None:
        inventory = self.inventory()
        with self.assertRaisesRegex(
            NativeAnimationInventoryError,
            "sourceStartMs escapes native clip",
        ):
            validate_native_source_window(
                inventory,
                clip_name="attack",
                source_start_ms=3667,
                play_ms=0,
                play_rate=1.0,
            )

    def test_rejects_missing_clip_and_nonfinite_play_rate(self) -> None:
        inventory = self.inventory()
        with self.assertRaisesRegex(
            NativeAnimationInventoryError,
            "absent from the product model composition",
        ):
            validate_native_source_window(
                inventory,
                clip_name="missing",
                source_start_ms=0,
                play_ms=1,
                play_rate=1.0,
            )
        with self.assertRaisesRegex(
            NativeAnimationInventoryError,
            "playRate must be finite and positive",
        ):
            validate_native_source_window(
                inventory,
                clip_name="attack",
                source_start_ms=0,
                play_ms=1,
                play_rate=math.nan,
            )


class RepositoryValtanNativeAnimationInventoryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        # Deliberately no SkipTest: an unavailable product asset must not turn
        # canonical source-window admission into a silent pass.
        cls.inventory = load_valtan_composite_animation_inventory(ROOT)

    def test_repository_product_composition_is_27_plus_146_clips(self) -> None:
        self.assertEqual(
            [source.clip_count for source in self.inventory.sources], [27, 146]
        )
        self.assertEqual(len(self.inventory.clips), 173)
        self.assertEqual(
            [source.asset_id for source in self.inventory.sources],
            [
                "Character/Valtan/MN_RPBF_01.wmodel",
                "Character/Valtan/AnimSets/MN_RPBF_01_AnimSet.wmodel",
            ],
        )

    def test_current_presentation_172_occurrences_fit_native_windows(self) -> None:
        presentation = json.loads(
            (ROOT / "Data/Valtan/Valtan.presentation.json").read_text(
                encoding="utf-8-sig"
            )
        )
        report = validate_valtan_presentation_native_windows(
            presentation, self.inventory
        )
        self.assertEqual(report.occurrence_count, 172)
        self.assertEqual(report.unique_clip_count, 75)
        self.assertEqual(report.native_remainder_occurrence_count, 45)

    def test_final_pipeline_gate_uses_the_same_173_clip_inventory(self) -> None:
        presentation = json.loads(
            (ROOT / "Data/Valtan/Valtan.presentation.json").read_text(
                encoding="utf-8-sig"
            )
        )
        report = pipeline.validate_valtan_native_animation_source(
            ROOT, presentation
        )
        self.assertEqual(report["inventoryClipCount"], 173)
        self.assertEqual(report["occurrenceCount"], 172)

        invalid = copy.deepcopy(presentation)
        occurrence = next(
            occurrence
            for pattern in invalid["patterns"]
            for stage in pattern["stages"]
            for occurrence in stage["animation"].get("occurrences", [])
            if occurrence["playMs"] > 0
        )
        native = self.inventory.clips[occurrence["clip"]]
        occurrence["sourceStartMs"] = 0
        occurrence["playMs"] = native.rounded_native_duration_ms + 1
        with self.assertRaisesRegex(
            pipeline.PipelineError,
            "native Animation source admission failed.*playMs exceeds",
        ):
            pipeline.validate_valtan_native_animation_source(ROOT, invalid)

    def test_all_mutating_publish_paths_gate_before_stage_or_projection(self) -> None:
        repository = inspect.getsource(
            pipeline.build_repository_product_projection
        )
        self.assertLess(
            repository.index("validate_valtan_native_animation_source"),
            repository.index("join_v2_authoring"),
        )
        save = inspect.getsource(pipeline.save_authoring)
        self.assertLess(
            save.index("validate_valtan_native_animation_source"),
            save.index("stage.mkdir()"),
        )
        publish = inspect.getsource(pipeline._publish_candidate_under_admission)
        self.assertLess(
            publish.index("validate_valtan_native_animation_source"),
            publish.index("outputs = project_v2_products"),
        )
        draft = inspect.getsource(pipeline.validate_draft_patch)
        self.assertLess(
            draft.index("validate_valtan_native_animation_source"),
            draft.index("projected = project_v2_products"),
        )
        projection = inspect.getsource(promotion.validate_and_project)
        self.assertLess(
            projection.index("validate_valtan_native_animation_source"),
            projection.index("joined = pipeline.join_v2_authoring"),
        )
        canonical_commit = inspect.getsource(
            promotion.commit_typed_authoring_patch
        )
        self.assertIn("outputs = validate_and_project", canonical_commit)

    def test_reference_clipcuts_cannot_be_copied_to_exact_source_play_ms(self) -> None:
        sequence_rows: dict[tuple[int, int], tuple[str, list[str]]] = {}
        for line in (
            ROOT / "Data/Animation/Reference/Valtan/Valtan.clipseq"
        ).read_text(encoding="utf-8-sig", errors="replace").splitlines()[1:]:
            match = re.fullmatch(
                r'(\d+)\s+"[^"]*"\s+seq=(\d+)\s+mode=(\S+)\s+clips="([^"]*)"',
                line,
            )
            self.assertIsNotNone(match, line)
            assert match is not None
            sequence_rows[(int(match[1]), int(match[2]))] = (
                match[3],
                match[4].split(","),
            )
        cut_rows: dict[tuple[int, int], list[int]] = {}
        for line in (
            ROOT / "Data/Animation/Reference/Valtan/Valtan.clipcuts"
        ).read_text(encoding="utf-8-sig").splitlines()[1:]:
            match = re.fullmatch(
                r'(\d+)\s+seq=(\d+)\s+cuts="([^"]*)"', line
            )
            self.assertIsNotNone(match, line)
            assert match is not None
            cut_rows[(int(match[1]), int(match[2]))] = [
                round(float(value) * 1000.0) for value in match[3].split(",")
            ]
        self.assertEqual(len(sequence_rows), 265)
        self.assertEqual(sequence_rows.keys(), cut_rows.keys())

        affected_sequences: set[tuple[int, int]] = set()
        overruns: list[tuple[tuple[int, int], str, int, int]] = []
        positive_slot_count = 0
        for key, (_mode, clip_names) in sequence_rows.items():
            cuts = cut_rows[key]
            self.assertEqual(len(clip_names), len(cuts), key)
            for clip_name, cut_ms in zip(clip_names, cuts, strict=True):
                if cut_ms <= 0:
                    continue
                positive_slot_count += 1
                native = self.inventory.clips.get(clip_name)
                self.assertIsNotNone(native, clip_name)
                assert native is not None
                if cut_ms > native.rounded_native_duration_ms:
                    affected_sequences.add(key)
                    overruns.append(
                        (
                            key,
                            clip_name,
                            cut_ms,
                            native.rounded_native_duration_ms,
                        )
                    )

        self.assertEqual(positive_slot_count, 1998)
        self.assertEqual(len(affected_sequences), 146)
        self.assertEqual(len(overruns), 398)
        self.assertIn(
            ((420610, 0), "mesh_att_battle_8_01_loop", 5000, 333),
            overruns,
        )


if __name__ == "__main__":
    unittest.main()
