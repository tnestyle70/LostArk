#!/usr/bin/env python3
"""Focused contract for promoted manual Pattern Stage/Sequence authoring."""

from __future__ import annotations

import copy
import json
import pathlib
import sys
import unittest


sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import valtan_tuning_pipeline as pipeline


ROOT = pathlib.Path(__file__).resolve().parents[2]
BALANCE_H = ROOT / "Client/Public/BalanceTool.h"
BALANCE_CPP = ROOT / "Client/Private/BalanceTool.cpp"
WORKBENCH_CPP = ROOT / "Client/Private/ActionCompositionWorkbench.cpp"


def pattern(master: dict, pattern_id: str) -> dict:
    return next(row for row in master["patterns"] if row["patternId"] == pattern_id)


def stage(owner: dict, stage_id: str) -> dict:
    return next(row for row in owner["stages"] if row["stageId"] == stage_id)


class ValtanManualStageAuthoringContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.docs = pipeline.load_pipeline_documents(ROOT)
        cls.source_revision = pipeline.source_manifest(ROOT)["sourceManifestId"]
        cls.master = pipeline.join_v2_authoring(
            cls.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            cls.docs[pipeline.PRESENTATION_AUTHORING_REL],
            cls.docs[pipeline.WORLD_SET_REL],
            cls.docs[pipeline.COMBAT_AUTHORING_REL],
        )

    def patch(self, operations: list[dict], owner: dict | None = None) -> dict:
        candidate, _, _, count = pipeline.apply_draft_patch(
            self.master if owner is None else owner,
            copy.deepcopy(self.docs[pipeline.BOSS_PROFILES_REL]),
            copy.deepcopy(self.docs[pipeline.DAMAGE_REL]),
            {
                "schema": pipeline.DRAFT_PATCH_SCHEMA,
                "formatVersion": 1,
                "sourceRevision": self.source_revision,
                "operations": operations,
            },
            self.source_revision,
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        self.assertEqual(len(operations), count)
        return candidate

    @staticmethod
    def authored_animation() -> dict:
        return {
            "endPolicy": "HOLD_LAST_POSE",
            "repeatCount": 1,
            "occurrences": [
                {
                    "clipOccurrenceId": "valtan.sequence.charge.step-01.composition.02",
                    "clip": "mesh_att_battle_9_01_loop",
                    "mappingBasis": "PROJECT_AUTHORED",
                    "sourceStartMs": 300,
                    "playMs": 600,
                    "playRate": 1.0,
                    "repeatUntilStageEnd": False,
                },
                {
                    "clipOccurrenceId": "valtan.sequence.charge.step-01.clip-01",
                    "clip": "mesh_att_battle_9_01_start",
                    "mappingBasis": "PROJECT_AUTHORED",
                    "sourceStartMs": 0,
                    "playMs": 500,
                    "playRate": 1.0,
                    "repeatUntilStageEnd": False,
                },
            ],
        }

    def composed_operations(self) -> list[dict]:
        return [
            {
                "op": "SET_STAGE_KIND",
                "patternId": "VALTAN_CHARGE",
                "stageId": "STEP_01",
                "stageKind": "WINDUP",
            },
            {
                "op": "SET_STAGE_KIND",
                "patternId": "VALTAN_CHARGE",
                "stageId": "STEP_03",
                "stageKind": "GROGGY",
            },
            {
                "op": "SET_STAGE_DURATION",
                "patternId": "VALTAN_CHARGE",
                "stageId": "STEP_01",
                "durationMs": 2200,
            },
            {
                "op": "SET_STAGE_ANIMATION",
                "patternId": "VALTAN_CHARGE",
                "stageId": "STEP_01",
                "animation": self.authored_animation(),
            },
            {
                "op": "SET_STAGE_COUNTER_WINDOW",
                "patternId": "VALTAN_CHARGE",
                "stageId": "STEP_01",
                "enabled": True,
                "successStageId": "STEP_03",
                "successActionId": "valtan.sequence.charge.step-03",
            },
        ]

    def test_manual_stage_sequence_gap_kind_and_counter_project_together(self) -> None:
        original = pattern(self.master, "VALTAN_CHARGE")
        original_identity = [
            (row["stageId"], row["actionId"], row["defaultNextActionId"])
            for row in original["stages"]
        ]
        candidate = self.patch(self.composed_operations())
        authored = pattern(candidate, "VALTAN_CHARGE")
        self.assertEqual(
            self.master["decisionModel"]["manualAuditions"],
            candidate["decisionModel"]["manualAuditions"],
            "composition must not promote or rewrite AUDITION_ONLY selection ownership",
        )
        for field in (
            "actionId",
            "entryActionId",
            "targetPolicy",
            "aimPolicy",
            "eligibility",
            "sourceActionIds",
            "presentationSources",
        ):
            self.assertEqual(original[field], authored[field])
        self.assertEqual(
            original_identity,
            [
                (row["stageId"], row["actionId"], row["defaultNextActionId"])
                for row in authored["stages"]
            ],
            "typed composition must preserve stable Stage/Action IDs and topology",
        )
        source = stage(authored, "STEP_01")
        target = stage(authored, "STEP_03")
        self.assertEqual("WINDUP", source["stageKind"])
        self.assertEqual("GROGGY", target["stageKind"])
        self.assertEqual(2200, source["durationMs"])
        self.assertEqual(
            [
                "mesh_att_battle_9_01_loop",
                "mesh_att_battle_9_01_start",
            ],
            [row["clip"] for row in source["animation"]["occurrences"]],
        )
        self.assertEqual(
            [300, 0],
            [row["sourceStartMs"] for row in source["animation"]["occurrences"]],
        )
        self.assertTrue(
            all(
                row["mappingBasis"] == "SOURCE_REVIEWED_DELTA"
                for row in source["animation"]["occurrences"]
            )
        )
        self.assertEqual(
            "SOURCE_REVIEWED_DELTA",
            source["effectCues"][0]["mappingBasis"],
            "a retained cue must stay joined to its retagged occurrence",
        )
        self.assertEqual(
            [{"outcome": "COUNTER_HIT", "nextActionId": target["actionId"]}],
            [row for row in source["branches"] if row["outcome"] == "COUNTER_HIT"],
        )

        gameplay, presentation = pipeline.split_v2_authoring(
            candidate,
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        rejoined = pipeline.join_v2_authoring(
            gameplay,
            presentation,
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        pipeline.validate_manual_audition_animation_lineage(
            rejoined,
            pipeline.read_json(ROOT / pipeline.DEBUG_PRESENTATION_REL),
            pipeline.read_json(ROOT / pipeline.ANIMATION_PROMOTION_MANIFEST_REL),
        )
        outputs = pipeline.project_v2_products(ROOT, self.docs, rejoined)
        bindings = json.loads(outputs[pipeline.BINDINGS_REL])
        binding = next(
            row
            for row in bindings["bindings"]
            if row["actionId"] == "valtan.sequence.charge.step-01"
        )
        self.assertEqual(
            ["mesh_att_battle_9_01_loop", "mesh_att_battle_9_01_start"],
            [row["clip"] for row in binding["clips"]],
        )
        encounter = json.loads(outputs[pipeline.ENCOUNTER_REL])
        product = next(
            row
            for row in encounter["patterns"]
            if row["patternId"] == "VALTAN_CHARGE"
        )
        product_source = stage(product, "STEP_01")
        self.assertEqual("WINDUP", product_source["stageKind"])
        self.assertEqual(2200, product_source["durationMs"])

    def test_invalid_kind_non_manual_kind_and_partial_failure_preserve_input(self) -> None:
        cases = (
            {
                "op": "SET_STAGE_KIND",
                "patternId": "VALTAN_CHARGE",
                "stageId": "STEP_01",
                "stageKind": "RECOVERY",
            },
            {
                "op": "SET_STAGE_KIND",
                "patternId": "VALTAN_HIGH_JUMP",
                "stageId": "AIRBORNE",
                "stageKind": "WINDUP",
            },
        )
        for operation in cases:
            before = copy.deepcopy(self.master)
            with self.subTest(operation=operation), self.assertRaises(
                pipeline.DraftPatchError
            ):
                self.patch([operation], before)
            self.assertEqual(self.master, before)

        before = copy.deepcopy(self.master)
        operations = [
            {
                "op": "SET_STAGE_DURATION",
                "patternId": "VALTAN_CHARGE",
                "stageId": "STEP_01",
                "durationMs": 2200,
            },
            cases[1],
        ]
        with self.assertRaises(pipeline.DraftPatchError):
            self.patch(operations, before)
        self.assertEqual(self.master, before)

    def test_linked_counter_stage_kinds_cannot_be_left_dangling(self) -> None:
        linked = self.patch(self.composed_operations())
        cases = (
            (
                {
                    "op": "SET_STAGE_KIND",
                    "patternId": "VALTAN_CHARGE",
                    "stageId": "STEP_01",
                    "stageKind": "ACTIVE",
                },
                "COUNTER_SOURCE_KIND_LOCKED",
            ),
            (
                {
                    "op": "SET_STAGE_KIND",
                    "patternId": "VALTAN_CHARGE",
                    "stageId": "STEP_03",
                    "stageKind": "ACTIVE",
                },
                "COUNTER_TARGET_KIND_LOCKED",
            ),
        )
        for operation, error_code in cases:
            before = copy.deepcopy(linked)
            with self.subTest(operation=operation), self.assertRaises(
                pipeline.DraftPatchError
            ) as raised:
                self.patch([operation], before)
            self.assertEqual(error_code, raised.exception.error_code)
            self.assertEqual(linked, before)

    def test_cpp_emitter_orders_counter_topology_before_field_patches(self) -> None:
        source = BALANCE_CPP.read_text(encoding="utf-8")
        emitter = source[
            source.index("bool Client::CBalanceTool::BuildValtanDraftPatch(") :
            source.index("bool Client::CBalanceTool::RunValtanDraftCommand(")
        ]
        phase_tokens = (
            "counterDisableOperations.begin(), counterDisableOperations.end()",
            "manualStagePreCounterTopologyOperations.begin()",
            "stageRoleOperations.begin(), stageRoleOperations.end()",
            "counterEnableOperations.begin(), counterEnableOperations.end()",
            "manualStagePostCounterTopologyOperations.begin()",
            "operations.begin(), operations.end()",
        )
        positions = [emitter.index(token) for token in phase_tokens]
        self.assertEqual(sorted(positions), positions)

        # This is the exact topology order emitted for a newly promoted
        # source Stage that precedes its Groggy target in Stage order.
        emitted_topology = [
            operation
            for kind in ("SET_STAGE_KIND", "SET_STAGE_COUNTER_WINDOW")
            for operation in self.composed_operations()
            if operation["op"] == kind
        ]
        candidate = self.patch(emitted_topology)
        authored = pattern(candidate, "VALTAN_CHARGE")
        self.assertEqual("WINDUP", stage(authored, "STEP_01")["stageKind"])
        self.assertEqual("GROGGY", stage(authored, "STEP_03")["stageKind"])
        self.assertEqual(
            "valtan.sequence.charge.step-03",
            next(
                branch["nextActionId"]
                for branch in stage(authored, "STEP_01")["branches"]
                if branch["outcome"] == "COUNTER_HIT"
            ),
        )

    def test_workbench_append_uses_a_free_stable_occurrence_after_delete(self) -> None:
        source = WORKBENCH_CPP.read_text(encoding="utf-8")
        allocator = source[
            source.index("std::string BuildNextCompositionSlotId(") :
            source.index("bool_t ComputeExactAnimationWallMs(")
        ]
        self.assertIn("ordinal <= Slots.size()", allocator)
        self.assertIn("Slot.clipOccurrenceId == candidate", allocator)
        append = source[
            source.index("bool_t Client::CActionCompositionWorkbench::Apply_SelectedSequenceToStage(") :
            source.index("bool_t Client::CActionCompositionWorkbench::Seek_EffectivePreview(")
        ]
        self.assertIn("BuildNextCompositionSlotId(", append)
        self.assertNotIn(
            "BuildCompositionSlotId(\n\t\t\t\tPattern.strPatternId",
            append,
        )

        # A middle delete leaves .01/.03 with size two.  Size-based allocation
        # reproduced .03; the bounded free-suffix scan selects .02.
        existing = {
            "VALTAN_CHARGE.STEP_01.composition.clip.01",
            "VALTAN_CHARGE.STEP_01.composition.clip.03",
        }
        candidate = next(
            f"VALTAN_CHARGE.STEP_01.composition.clip.{ordinal:02d}"
            for ordinal in range(1, len(existing) + 2)
            if f"VALTAN_CHARGE.STEP_01.composition.clip.{ordinal:02d}"
            not in existing
        )
        self.assertEqual("VALTAN_CHARGE.STEP_01.composition.clip.02", candidate)
        self.assertNotIn(candidate, existing)

        self.assertIn(
            '"Release Velocity (m/s)", &Action.fSpeedMps, 0.05f, 0.f, 50.f',
            source,
        )
        self.assertIn(
            '"Release Duration (ms)", &iReleaseDuration, 5.f, 0, 5000',
            source,
        )

    def test_last_sequence_slot_remove_becomes_manual_animation_none(self) -> None:
        workbench = WORKBENCH_CPP.read_text(encoding="utf-8")
        details = workbench[
            workbench.index(
                "void Client::CActionCompositionWorkbench::Render_AnimationStageDetails("
            ) : workbench.index(
                "void Client::CActionCompositionWorkbench::Render_Details("
            )
        ]
        self.assertIn("if (iRemove < Draft.animationSlots.size())", details)
        remove_block = details[
            details.index("if (iRemove < Draft.animationSlots.size())") :
            details.index("else if (iMoveUp", details.index("if (iRemove <"))
        ]
        self.assertNotIn("Draft.animationSlots.size() > 1u", remove_block)
        self.assertIn('Draft.animationEndPolicy = "NONE"', details)
        self.assertIn("Draft.animationRepeatCount = Draft.animationSlots.empty() ? 0u", details)
        self.assertIn("Animation Mode: NONE", details)

        balance = BALANCE_CPP.read_text(encoding="utf-8")
        setter = balance[
            balance.index("bool Client::CBalanceTool::Set_ValtanStageDraft(") :
            balance.index("bool Client::CBalanceTool::Get_ValtanCounterWindowDraft(")
        ]
        for token in (
            "const bool bAnimationNone = candidate.animationSlots.empty()",
            "!pattern->bManualServerAudition || isWaitStage",
            '"NONE" != candidate.animationEndPolicy',
            "0u != candidate.animationRepeatCount",
            "stage->ClipOccurrences.clear()",
            'stage->strAnimationEndPolicy = "NONE"',
            "stage->iAuthoringRepeatCount = 0u",
            "stage->bSuppressAnimation = true",
        ):
            self.assertIn(token, setter)
        emitter = balance[
            balance.index("bool Client::CBalanceTool::BuildValtanDraftPatch(") :
            balance.index("bool Client::CBalanceTool::RunValtanDraftCommand(")
        ]
        self.assertIn(r'\"animation\": { \"mode\": \"NONE\" }', emitter)

        pipeline_source = (
            ROOT / "Tools/ValtanPipeline/valtan_tuning_pipeline.py"
        ).read_text(encoding="utf-8")
        self.assertIn('if set(animation) == {"mode"}:', pipeline_source)
        self.assertIn(
            'stage["animation"] = {"mode": ANIMATION_MODE_NONE}',
            pipeline_source,
        )
        self.assertIn("has_reviewed_none_delta", pipeline_source)

    def test_balance_backend_exposes_manual_capability_and_rejects_stale_mutation(self) -> None:
        header = BALANCE_H.read_text(encoding="utf-8")
        source = BALANCE_CPP.read_text(encoding="utf-8")
        for token in (
            "stageKindEditable",
            "SET_STAGE_KIND",
            "MANUAL_SERVER_AUDITION Stage admits ACTIVE, WINDUP, or GROGGY",
            "draft.durationEditable = true",
            "!stage.bSuppressAnimation && !stage.ClipOccurrences.empty()",
            "disable this Stage's Counter window before changing its WINDUP kind",
            "disable every Counter window targeting this GROGGY Stage before changing its kind",
        ):
            self.assertIn(token, header + source)
        gate = "Require_ValtanAuthoringAdmission"
        self.assertIn(gate, header)
        for function in (
            "Set_ValtanStageDraft",
            "Set_ValtanCounterWindowDraft",
            "Set_ValtanHighJumpAxeCountDraft",
        ):
            body = source[source.index(f"bool Client::CBalanceTool::{function}(") :]
            body = body[: body.index("\n}")]
            self.assertIn(gate, body)
        self.assertIn("stale-preserved graph is display-only", source)
        render = source[
            source.index("void Client::CBalanceTool::RenderValtanPatternAuthoring()") :
            source.index("void Client::CBalanceTool::RenderValtanDecisionTrace(")
        ]
        self.assertIn("STALE PRESERVED / READ ONLY", render)
        self.assertIn("ImGui::BeginDisabled(!bAuthoringAdmitted)", render)


if __name__ == "__main__":
    unittest.main(verbosity=2)
