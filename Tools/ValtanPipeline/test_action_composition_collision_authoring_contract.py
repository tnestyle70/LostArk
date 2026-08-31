#!/usr/bin/env python3
from __future__ import annotations

import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PIPELINE_ROOT = ROOT / "Tools/ValtanPipeline"
if str(PIPELINE_ROOT) not in sys.path:
    sys.path.insert(0, str(PIPELINE_ROOT))

import valtan_tuning_pipeline as pipeline  # noqa: E402


def read_text(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8-sig")


def between(text: str, start: str, end: str) -> str:
    start_index = text.index(start)
    end_index = text.index(end, start_index + len(start))
    return text[start_index:end_index]


class ActionCompositionCollisionAuthoringContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.workbench = read_text(
            "Client/Private/ActionCompositionWorkbench.cpp"
        )
        cls.balance = read_text("Client/Private/BalanceTool.cpp")
        cls.pipeline_source = read_text(
            "Tools/ValtanPipeline/valtan_tuning_pipeline.py"
        )
        cls.gameplay_details = between(
            cls.workbench,
            "void Client::CActionCompositionWorkbench::Render_GameplayStageDetails(",
            "void Client::CActionCompositionWorkbench::Render_AnimationStageDetails(",
        )

    def test_details_keep_collision_directions_and_lifetimes_explicit(self) -> None:
        for marker in (
            'ImGui::SeparatorText("Stage Hit (Boss -> Player)")',
            'ImGui::SeparatorText("Counter Hurt Proxy (Player -> Boss)")',
            'ImGui::SeparatorText("Status & Gauge (Non-spatial)")',
            "Effect geometry is presentation only and never becomes hit authority.",
            "Each scheduled time is an instantaneous Server pulse.",
            "Lifetime: whole WINDUP Stage.",
            "It never damages or captures a player.",
        ):
            self.assertIn(marker, self.gameplay_details)

        self.assertIn(
            "Status value and ENTER/EXIT timing remain read-only",
            self.gameplay_details,
        )
        self.assertNotIn('"Status Window (ms)"', self.gameplay_details)

    def test_explicit_offsets_are_editable_and_remain_chronological(self) -> None:
        for marker in (
            '"##ExplicitHitOffsetMs"',
            'ImGui::SmallButton("Add Hit Offset")',
            'ImGui::SmallButton("Delete")',
            "std::lower_bound(",
            "Ordered.insert(InsertAt, iCandidate)",
            "Draft.hitOffsetsMs.size() < 64u",
            "Draft.hitOffsetsMs.back() < iMaximumOffsetMs",
            "Draft.hitCount = static_cast<uint32_t>(",
            "Draft.hitDelayMs = 0u",
            "Draft.hitIntervalMs = 0u",
            "SetValtanStageDraftWithSoundDependencyAdmission(",
        ):
            self.assertIn(marker, self.gameplay_details)

        self.assertIn(
            "changing a time reorders it chronologically",
            self.gameplay_details,
        )
        self.assertNotIn(
            "!portalRushMotion && hitOffsetsChanged",
            self.balance,
        )
        self.assertIn("std::adjacent_find(", self.balance)
        self.assertIn('"EXPLICIT_OFFSETS"', self.balance)
        self.assertIn("SET_STAGE_HIT", self.balance)

    def test_stage_hit_trigger_result_uses_typed_damage_or_grab(self) -> None:
        for marker in (
            'const char_t* const TriggerResults[] = { "Damage", "Grab" }',
            '"Trigger Result", &iTriggerResult',
            'Draft.playerResponse = "CAPTURE"',
            'Draft.attachmentSlot = "BOSS_LEFT_HAND"',
            "Draft.pushRangeM = 0.0",
            "Draft.pushMs = 0u",
            "Draft.knockdown = false",
            "Draft.downMs = 0u",
            "ImGui::BeginDisabled(bCaptureCollider)",
            "Removing Capture requires the whole typed grab topology transaction.",
            "a rejected Save keeps its exact reason.",
        ):
            self.assertIn(marker, self.gameplay_details)

        for marker in (
            "const bool responseChanged =",
            '"DAMAGE" == current.playerResponse',
            '"CAPTURE" == candidate.playerResponse',
            '"BOSS_LEFT_HAND" == candidate.attachmentSlot && ownsRelease',
            "Removing Capture requires the whole grab topology transaction.",
            "stage->strPlayerResponse = candidate.playerResponse",
            "stage->strAttachmentSlot = candidate.attachmentSlot",
            "stage.strPlayerResponse != loadedStage->strPlayerResponse",
            "stage.strAttachmentSlot != loadedStage->strAttachmentSlot",
        ):
            self.assertIn(marker, self.balance)

    def test_push_range_matches_client_and_server_signed_contract(self) -> None:
        exact_call = (
            'number(hit["pushRangeM"], '
            'f"{pattern_id}/{stage_id}.pushRangeM", -20, 20)'
        )
        self.assertIn(exact_call, self.pipeline_source)
        self.assertIn(
            'number(hit["pushRangeM"], f"{hit_context}.pushRangeM", -20, 20)',
            self.pipeline_source,
        )
        self.assertNotIn(
            'number(hit["pushRangeM"], '
            'f"{hit_context}.pushRangeM", 0.0, 100000.0)',
            self.pipeline_source,
        )

        for value in (-20.0, -1.25, 0.0, 1.25, 20.0):
            with self.subTest(value=value):
                self.assertEqual(
                    value,
                    pipeline.number(value, "test.pushRangeM", -20, 20),
                )
        for value in (-20.001, 20.001):
            with self.subTest(value=value):
                with self.assertRaises(pipeline.PipelineError):
                    pipeline.number(value, "test.pushRangeM", -20, 20)


if __name__ == "__main__":
    unittest.main()
