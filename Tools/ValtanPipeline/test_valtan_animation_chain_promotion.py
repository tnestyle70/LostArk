#!/usr/bin/env python3
from __future__ import annotations

import argparse
import hashlib
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import promote_valtan_animation_chains as promotion


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


class ValtanAnimationChainPromotionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.root = REPOSITORY_ROOT.resolve()

    def test_reviewed_chain_closure_and_stable_ids(self) -> None:
        gameplay, presentation, receipt = promotion.build_candidates(self.root)
        self.assertEqual(27, len(gameplay["patterns"]))
        self.assertEqual(27, len(presentation["patterns"]))
        self.assertEqual(20, receipt["patternCount"])
        self.assertEqual(94, receipt["stageCount"])
        self.assertEqual(
            94,
            sum(len(pattern["occurrences"]) for pattern in receipt["patterns"]),
        )
        first = receipt["patterns"][0]
        self.assertEqual("VALTAN_SEQUENCE_CENTER_SIX_PIZZA_CHARGE", first["patternId"])
        self.assertEqual("STEP_01", first["occurrences"][0]["targetStageId"])
        self.assertEqual(
            "valtan.sequence.center-six-pizza-charge.step-01.clip-01",
            first["occurrences"][0]["targetClipOccurrenceId"],
        )
        self.assertEqual(
            "VALTAN_SEQUENCE_WARP_JUMP_FOUR_HAND_TWOHAND_ROAR_ROAR_DEAD",
            receipt["patterns"][-1]["patternId"],
        )

    def test_production_closure_counts_follow_reviewed_manifest_and_debug(self) -> None:
        promotions = [{"sourceChainId": "phase-three-a"}, {"sourceChainId": "phase-three-b"}]
        chains = [
            {"animation": {"occurrences": [{}, {}]}},
            {"animation": {"occurrences": [{}]}},
        ]
        self.assertEqual(
            (2, 3), promotion._reviewed_closure_counts(promotions, chains)
        )

        future_receipt = {
            "patternCount": 21,
            "stageCount": 95,
            "patterns": [{"occurrences": []}],
        }
        with mock.patch.object(
            promotion,
            "build_candidates",
            return_value=({}, {}, future_receipt),
        ), mock.patch.object(
            promotion,
            "validate_and_project",
            return_value={},
        ):
            result = promotion.run(self.root, "Validate")
        self.assertEqual(21, result["patternCount"])
        self.assertEqual(95, result["stageCount"])

    def test_native_explicit_and_loop_durations_are_frozen(self) -> None:
        _gameplay, _presentation, receipt = promotion.build_candidates(self.root)
        occurrences = {
            row["sourceClipOccurrenceId"]: row
            for pattern in receipt["patterns"]
            for row in pattern["occurrences"]
        }
        native = occurrences["valtan.debug.center-six-pizza-charge.clip.01"]
        self.assertEqual("NATIVE_WMODEL", native["resolution"])
        self.assertEqual("EXACT", native["endPolicy"])
        self.assertGreater(native["productPlayMs"], 0)
        self.assertEqual(native["nativeSourceMs"], native["stageDurationMs"])

        loop = occurrences["valtan.debug.center-six-pizza-charge.clip.06"]
        self.assertEqual("EXPLICIT_WALL_LOOP", loop["resolution"])
        self.assertEqual(8000, loop["stageDurationMs"])
        self.assertEqual(0, loop["productPlayMs"])
        self.assertEqual("LOOP_TO_STAGE_END", loop["endPolicy"])

        exact = next(
            row
            for row in occurrences.values()
            if row["resolution"] == "EXPLICIT_WALL_EXACT"
        )
        self.assertEqual("EXACT", exact["endPolicy"])
        self.assertGreater(exact["productPlayMs"], 0)

    def test_all_six_reviewed_clip_aliases_are_explicit(self) -> None:
        _gameplay, _presentation, receipt = promotion.build_candidates(self.root)
        aliases = {
            row["sourceClip"]: row["resolvedClip"]
            for pattern in receipt["patterns"]
            for row in pattern["occurrences"]
            if row["aliasApplied"]
        }
        self.assertEqual(
            {
                "att_battle_2_03": "mesh_att_battle_2_03",
                "att_battle_19_02": "mesh_att_battle_19_02",
                "att_battle_19_04": "mesh_att_battle_19_04",
                "att_battle_20_02": "mesh_att_battle_20_02",
                "att_battle_20_03": "mesh_att_battle_20_03",
                "att_battle_20_04": "mesh_att_battle_20_04",
            },
            aliases,
        )

    def test_atomic_commit_rolls_back_every_replaced_target(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            targets = {
                root / "one.json": b"new-one\n",
                root / "two.json": b"new-two\n",
                root / "three.json": b"new-three\n",
            }
            for index, path in enumerate(targets, start=1):
                path.write_bytes(f"old-{index}\n".encode("ascii"))
            before = {path: path.read_bytes() for path in targets}
            with self.assertRaises(promotion.PromotionError):
                promotion._atomic_commit(targets, inject_failure_after=2)
            self.assertEqual(before, {path: path.read_bytes() for path in targets})
            self.assertFalse(any(root.glob("*.tmp")))
            self.assertFalse(any(root.glob(".*.tmp")))

    def test_validate_mode_does_not_mutate_repository_products(self) -> None:
        tracked = (
            promotion.GAMEPLAY_REL,
            promotion.PRESENTATION_REL,
            "Data/Encounters/Valtan/ValtanEncounter.json",
            "Data/Animation/Authored/Valtan/Valtan.patternbindings.json",
            "Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json",
        )
        before = {relative: sha256(self.root / relative) for relative in tracked}
        result = promotion.run(self.root, "Validate")
        self.assertEqual(20, result["patternCount"])
        self.assertEqual(94, result["stageCount"])
        self.assertEqual(before, {relative: sha256(self.root / relative) for relative in tracked})


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--repository-root", type=Path, default=REPOSITORY_ROOT)
    arguments, remaining = parser.parse_known_args()
    REPOSITORY_ROOT = arguments.repository_root.resolve()
    program = unittest.main(argv=[sys.argv[0], *remaining], verbosity=2, exit=False)
    raise SystemExit(0 if program.result.wasSuccessful() else 1)
