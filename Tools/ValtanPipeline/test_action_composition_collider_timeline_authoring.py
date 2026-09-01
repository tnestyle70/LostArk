#!/usr/bin/env python3
from __future__ import annotations

import json
import shutil
import subprocess
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PIPELINE_ROOT = ROOT / "Tools/ValtanPipeline"
if str(PIPELINE_ROOT) not in sys.path:
    sys.path.insert(0, str(PIPELINE_ROOT))

import test_valtan_canonical_typed_patch_transaction as canonical_fixture  # noqa: E402


WORKBENCH_CPP = ROOT / "Client/Private/ActionCompositionWorkbench.cpp"
WORKBENCH_H = ROOT / "Client/Public/ActionCompositionWorkbench.h"
ANIMATION_TOOL_CPP = ROOT / "Client/Private/Animation_Tool.cpp"
VALTAN_CPP = ROOT / "Client/Private/Valtan.cpp"


def function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for index in range(brace, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
    raise AssertionError(f"unterminated function: {signature}")


class ActionCompositionColliderTimelineAuthoringTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.source = WORKBENCH_CPP.read_text(encoding="utf-8-sig")
        cls.header = WORKBENCH_H.read_text(encoding="utf-8-sig")
        cls.animation_source = ANIMATION_TOOL_CPP.read_text(encoding="utf-8-sig")
        cls.valtan_source = VALTAN_CPP.read_text(encoding="utf-8-sig")

    def test_body_shift_preserves_spacing_and_clamps_to_stage_clock(self) -> None:
        translate = function_body(
            self.source,
            "bool_t TranslateValtanColliderSchedule(",
        )
        for token in (
            "-static_cast<int64_t>(iFirstMs)",
            "Draft.durationMs - 1u - iLastMs",
            "Draft.hitDelayMs = static_cast<uint32_t>(",
            "for (uint32_t& iOffsetMs : Draft.hitOffsetsMs)",
            "iOffsetMs) + iAppliedDeltaMs",
            "preserving the exact spacing",
        ):
            self.assertIn(token, translate)
        self.assertNotIn("Draft.hitIntervalMs =", translate)
        self.assertNotIn("Draft.hitCount =", translate)

    def test_left_and_right_edges_edit_only_endpoints_and_reject_one_pulse(self) -> None:
        resize = function_body(
            self.source,
            "bool_t ResizeValtanColliderScheduleEndpoint(",
        )
        for token in (
            "Draft.hitCount < 2u",
            "A one-pulse Server collider is instantaneous",
            "Offsets[1u] - 1u",
            "Offsets[Offsets.size() - 2u] + 1u",
            "const std::size_t iEndpointIndex = bResizeStart ?",
            "Offsets[iEndpointIndex] = iClampedEndpointMs",
            "Draft.hitOffsetsMs = std::move(Offsets)",
            "Draft.hitDelayMs = 0u",
            "Draft.hitIntervalMs = 0u",
        ):
            self.assertIn(token, resize)

        timeline = function_body(
            self.source,
            "void Client::CActionCompositionWorkbench::Render_Timeline(",
        )
        for token in (
            "bColliderSchedule",
            "bStartTrimHandleHovered",
            "fEndTrimHandleX",
            "m_iTimelineTrimSourceEndpointMs",
            "m_iTimelineTrimMouseStartMs",
            "TranslateValtanColliderSchedule(",
            "ResizeValtanColliderScheduleEndpoint(",
            "SetValtanStageDraftWithSoundDependencyAdmission(",
            "Refresh_PatternLocalPreviewAfterMutation(pPattern, Status)",
        ):
            self.assertIn(token, timeline)
        self.assertIn("TIMELINE_POINT_MINIMUM_WIDTH_PX", timeline)
        self.assertIn("fSemanticEndX", timeline)
        self.assertIn("m_bTimelineTrimStartEdge", self.header)

    def test_preview_restage_reads_fresh_effective_pattern_value(self) -> None:
        refresh = function_body(
            self.source,
            "void Client::CActionCompositionWorkbench::\nRefresh_PatternLocalPreviewAfterMutation(",
        )
        for token in (
            "m_pBalanceTool->Get_ValtanPatternDraft(",
            "VALTAN_PATTERN_VIEW EffectivePattern",
            "Play_EffectivePreview(EffectivePattern",
            "Seek_EffectivePreview(\n\t\t\tEffectivePattern",
            "boss-local debug collider",
        ):
            self.assertIn(token, refresh)
        self.assertNotIn("Play_EffectivePreview(*pPattern", refresh)

    def test_box_detail_sizes_are_clamped_and_staged_for_physical_save(self) -> None:
        details = function_body(
            self.source,
            "void Client::CActionCompositionWorkbench::Render_GameplayStageDetails(",
        )
        for token in (
            '"Outer Radius (m)"',
            '"Inner Radius (m)"',
            '"Angle (deg)"',
            '"Length (m)"',
            '"Half Width (m)"',
            "ImGuiSliderFlags_AlwaysClamp",
            "SameValtanColliderDraft(ColliderBaseline, Draft)",
            "SetValtanStageDraftWithSoundDependencyAdmission(",
            "Play/Restart restages the boss-local debug wire",
        ):
            self.assertIn(token, details)

    def test_arena_clone_debug_wire_uses_local_draft_and_boss_transform(self) -> None:
        stage_preview = function_body(
            self.valtan_source,
            "bool_t CValtan::Stage_LocalPatternAuthoringPreview(",
        )
        draw = function_body(
            self.valtan_source,
            "void CValtan::Draw_PatternHitAreaDebug() const",
        )
        for token in (
            "Area.fOuterRadius = Stage.fHitOuterRadius",
            "Area.fInnerRadius = Stage.fHitInnerRadius",
            "Area.fAngleDegrees = Stage.fHitAngleDegrees",
            "Area.fLength = Stage.fHitLength",
            "Area.fHalfWidth = Stage.fHitHalfWidth",
            "Area.HitOffsetsMs = Stage.HitOffsetsMs",
            "Area.iStageDurationMs = Stage.iDurationMs",
            "m_LocalPreviewHitAreaByActionId = std::move(StagedHitAreas)",
        ):
            self.assertIn(token, stage_preview)
        for token in (
            "m_LocalPreviewHitAreaByActionId",
            "PATTERN_HIT_COLOR_RGBA",
            "PATTERN_AUTHORING_GEOMETRY_COLOR_RGBA",
            "m_pTransformCom->Get_WorldMatrixPtr()",
            "const vector_t vLook",
            "CHitAreaWire::Draw(Root, Shape, iColor)",
        ):
            self.assertIn(token, draw)
        self.assertIn(
            "PreviewBoss->Stage_LocalPatternAuthoringPreview(Pattern, Status)",
            self.animation_source,
        )

    def test_public_wrapper_persists_offsets_and_geometry_to_physical_gameplay_json(
        self,
    ) -> None:
        # Reuse the canonical transaction fixture so this focused check gets the
        # same isolated Data/tools/model closure as the shipped public wrapper.
        fixture = canonical_fixture.ValtanCanonicalTypedPatchTransactionTests(
            "test_public_wrapper_noop_reports_not_activated"
        )
        fixture.setUp()
        try:
            patch_path = fixture.write_patch(
                "collider-timeline-save.json",
                fixture.repository_revision,
                [
                    {
                        "op": "SET_STAGE_HIT",
                        "patternId": "VALTAN_GROUND_ROAR",
                        "stageId": "STEP_01",
                        "hit": {
                            "shape": {
                                "kind": "CONE",
                                "angleDegrees": 110.0,
                                "lengthM": 14.0,
                            },
                            "schedule": {
                                "kind": "EXPLICIT_OFFSETS",
                                "offsetsMs": [250, 900],
                            },
                            "serverDamageProfileId": "damage.valtan.four-slash",
                            "pushRangeM": 2.0,
                            "pushMs": 150,
                            "knockdown": True,
                            "downMs": 1000,
                        },
                    }
                ],
            )
            powershell = shutil.which("powershell.exe") or shutil.which("powershell")
            self.assertIsNotNone(powershell)
            completed = subprocess.run(
                [
                    str(powershell),
                    "-NoProfile",
                    "-ExecutionPolicy",
                    "Bypass",
                    "-File",
                    str(fixture.wrapper),
                    "-Mode",
                    "CommitCanonicalDraft",
                    "-RepositoryRoot",
                    str(fixture.root),
                    "-AuthoringRoot",
                    "Intermediate/ValtanTuningAuthoring",
                    "-DraftPatchPath",
                    str(patch_path),
                ],
                cwd=fixture.root,
                env=fixture.environment,
                capture_output=True,
                text=True,
                encoding="utf-8",
                errors="replace",
                check=False,
            )
            self.assertEqual(
                0, completed.returncode, completed.stdout + completed.stderr
            )
            result = fixture.parse_command_result(completed)
            self.assertTrue(result["ok"])
            self.assertEqual(1, result["payload"]["operationCount"])

            gameplay = json.loads(
                (fixture.root / "Data/Valtan/Valtan.gameplay.json").read_text(
                    encoding="utf-8"
                )
            )
            stage = next(
                stage
                for pattern in gameplay["patterns"]
                if pattern["patternId"] == "VALTAN_GROUND_ROAR"
                for stage in pattern["stages"]
                if stage["stageId"] == "STEP_01"
            )
            self.assertEqual(
                {
                    "kind": "CONE",
                    "angleDegrees": 110.0,
                    "lengthM": 14.0,
                },
                stage["hit"]["shape"],
            )
            self.assertEqual(
                {"kind": "EXPLICIT_OFFSETS", "offsetsMs": [250, 900]},
                stage["hit"]["schedule"],
            )
        finally:
            fixture.tearDown()


if __name__ == "__main__":
    unittest.main()
