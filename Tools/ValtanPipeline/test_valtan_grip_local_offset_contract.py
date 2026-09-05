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

    def test_client_composes_the_grip_before_the_parts_update(self) -> None:
        header = VALTAN_HEADER.read_text(encoding="utf-8-sig")
        valtan = VALTAN_SOURCE.read_text(encoding="utf-8-sig")
        replication_header = REPLICATION_HEADER.read_text(encoding="utf-8-sig")
        replication = REPLICATION_SOURCE.read_text(encoding="utf-8-sig")
        character = CHARACTER_SOURCE.read_text(encoding="utf-8-sig")
        game_room = GAME_ROOM_SOURCE.read_text(encoding="utf-8-sig")
        # The only Client writer runs inside CCharacter::Update, after the
        # Server interpolation and before the parts compose their world.
        update = character[character.index("void CCharacter::Update(f32_t fTimeDelta)"):]
        update = update[: update.index("void CCharacter::Late_Update(")]
        self.assertLess(
            update.index("Update_NetworkTransform(fTimeDelta);"),
            update.index("Update_NetworkAttachmentTransform(fTimeDelta);"),
        )
        self.assertLess(
            update.index("Update_NetworkAttachmentTransform(fTimeDelta);"),
            update.index("__super::Update(fTimeDelta);"),
        )
        self.assertIn("PLAYER_ACTION_STATE::GRABBED == action", character)
        # Level-update-phase overwrites never render: the parts already composed.
        for forbidden in (
            "Update_PlayerAttachmentPresentations",
            "Stage_PlayerAttachmentPresentation",
            "bip001-l-hand",
            "m_PlayerAttachments",
        ):
            self.assertNotIn(forbidden, replication_header)
            self.assertNotIn(forbidden, replication)
        self.assertIn("character->Apply_NetworkAttachment(", replication)
        self.assertIn("character->Clear_NetworkAttachment();", replication)
        # Valtan owns the socket bone and the encounter-wide admitted grip.
        self.assertIn("public IPlayerHandGripSocketSource", header)
        self.assertIn("m_PlayerHandGripLocalOffset", header)
        self.assertIn('VALTAN_LEFT_HAND_BONE = "bip001-l-hand"', valtan)
        self.assertIn(
            "Reload_PlayerHandGripLocalOffset_WhileAdmitted(StepStatus)", valtan
        )
        self.assertIn("Get_BoneMatrix(VALTAN_LEFT_HAND_BONE)", valtan)
        # Server authority is unchanged.
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
