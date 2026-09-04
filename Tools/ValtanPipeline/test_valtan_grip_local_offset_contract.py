import copy
import json
import math
import unittest
from pathlib import Path

from Tools.ValtanPipeline import valtan_tuning_pipeline as pipeline


ROOT = Path(__file__).resolve().parents[2]
GAMEPLAY = ROOT / "Data/Valtan/Valtan.gameplay.json"
VALTAN_HEADER = ROOT / "Client/Public/Valtan.h"
VALTAN_SOURCE = ROOT / "Client/Private/Valtan.cpp"
REPLICATION_HEADER = ROOT / "Client/Public/ClientReplication.h"
REPLICATION_SOURCE = ROOT / "Client/Private/ClientReplication.cpp"
CHARACTER_SOURCE = ROOT / "Client/Private/Character.cpp"
GAME_ROOM_SOURCE = ROOT / "Server/Private/GameRoom.cpp"


def load_gameplay() -> dict:
    return json.loads(GAMEPLAY.read_text(encoding="utf-8"))


def capture_hits(document: dict) -> list[dict]:
    return [
        stage["hit"]
        for pattern in document["patterns"]
        for stage in pattern["stages"]
        if stage["hit"].get("playerResponse") == "CAPTURE"
    ]


class ValtanGripLocalOffsetContractTests(unittest.TestCase):
    def test_all_stable_capture_actions_project_the_authored_grip(self) -> None:
        document = load_gameplay()
        pipeline.validate_gameplay_authoring(document)
        hits = capture_hits(document)
        self.assertEqual(7, len(hits))
        expected = {"forwardM": 0.0, "upM": -0.9, "rightM": 0.0}
        for hit in hits:
            self.assertEqual(expected, hit["gripLocalOffset"])
            projected = pipeline._compile_hit(hit)
            self.assertEqual(expected, projected["gripLocalOffset"])
            self.assertEqual("CAPTURE", projected["playerResponse"])
            self.assertEqual("BOSS_LEFT_HAND", projected["attachmentSlot"])

    def test_capture_grip_is_deterministic_per_pattern(self) -> None:
        bindings = {}
        for pattern in load_gameplay()["patterns"]:
            grips = [
                stage["hit"]["gripLocalOffset"]
                for stage in pattern["stages"]
                if stage["hit"].get("playerResponse") == "CAPTURE"
            ]
            if not grips:
                continue
            self.assertTrue(all(grip == grips[0] for grip in grips))
            bindings[pattern["patternId"]] = grips[0]
        self.assertEqual(
            {"VALTAN_TRASH", "VALTAN_TRASH_CATCH_IF", "VALTAN_CATCH_BREATH"},
            set(bindings),
        )

    def test_client_never_composes_the_grip_on_a_hand_bone(self) -> None:
        header = VALTAN_HEADER.read_text(encoding="utf-8-sig")
        valtan = VALTAN_SOURCE.read_text(encoding="utf-8-sig")
        replication_header = REPLICATION_HEADER.read_text(encoding="utf-8-sig")
        replication = REPLICATION_SOURCE.read_text(encoding="utf-8-sig")
        character = CHARACTER_SOURCE.read_text(encoding="utf-8-sig")
        game_room = GAME_ROOM_SOURCE.read_text(encoding="utf-8-sig")
        for forbidden in (
            "Update_PlayerAttachmentPresentations",
            "Stage_PlayerAttachmentPresentation",
            "bip001-l-hand",
            "m_PlayerAttachments",
        ):
            self.assertNotIn(forbidden, replication_header)
            self.assertNotIn(forbidden, replication)
        for forbidden in (
            "m_PlayerHandGripLocalOffsetByActionId",
            "m_PlayerHandGripLocalOffsetByPatternId",
            "Try_Get_PlayerHandGripLocalOffset",
            "Reload_PlayerHandGripLocalOffsets_WhileAdmitted",
        ):
            self.assertNotIn(forbidden, header)
            self.assertNotIn(forbidden, valtan)
        self.assertIn("PLAYER_ACTION_STATE::GRABBED == action", character)
        self.assertIn("Update_PlayerAttachment(player, updateTick)", game_room)
        self.assertIn("player.fAttachmentLocalOffsetX * cosine", game_room)

    def test_missing_grip_rejects_without_mutating_committed_source(self) -> None:
        committed = load_gameplay()
        baseline = copy.deepcopy(committed)
        candidate = copy.deepcopy(committed)
        del capture_hits(candidate)[0]["gripLocalOffset"]
        with self.assertRaisesRegex(
            pipeline.PipelineError,
            "requires playerResponse, attachmentSlot, and gripLocalOffset together",
        ):
            pipeline.validate_gameplay_authoring(candidate)
        self.assertEqual(baseline, committed)

    def test_nonfinite_out_of_range_and_hidden_fields_are_rejected(self) -> None:
        for field, value in (
            ("forwardM", math.nan),
            ("upM", math.inf),
            ("rightM", 10.01),
        ):
            candidate = load_gameplay()
            capture_hits(candidate)[0]["gripLocalOffset"][field] = value
            with self.assertRaises(pipeline.PipelineError):
                pipeline.validate_gameplay_authoring(candidate)

        candidate = load_gameplay()
        capture_hits(candidate)[0]["gripLocalOffset"]["hidden"] = 1.0
        with self.assertRaisesRegex(pipeline.PipelineError, "fields mismatch"):
            pipeline.validate_gameplay_authoring(candidate)


if __name__ == "__main__":
    unittest.main()
