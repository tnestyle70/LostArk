#!/usr/bin/env python3
"""Focused source contract for manual Stage topology in Action Composition."""

from __future__ import annotations

import pathlib
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[2]
BALANCE_H = ROOT / "Client/Public/BalanceTool.h"
BALANCE_CPP = ROOT / "Client/Private/BalanceTool.cpp"
WORKBENCH_H = ROOT / "Client/Public/ActionCompositionWorkbench.h"
WORKBENCH_CPP = ROOT / "Client/Private/ActionCompositionWorkbench.cpp"
WORKBENCH_BLUEPRINT_CPP = (
    ROOT / "Client/Private/ActionCompositionWorkbench_Blueprint.cpp"
)


def function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for cursor in range(opening, len(source)):
        if source[cursor] == "{":
            depth += 1
        elif source[cursor] == "}":
            depth -= 1
            if depth == 0:
                return source[opening : cursor + 1]
    raise AssertionError(f"unterminated function: {signature}")


class ActionCompositionManualStageTopologyContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.balance_h = BALANCE_H.read_text(encoding="utf-8")
        cls.balance_cpp = BALANCE_CPP.read_text(encoding="utf-8")
        cls.workbench_h = WORKBENCH_H.read_text(encoding="utf-8")
        cls.workbench_cpp = WORKBENCH_CPP.read_text(encoding="utf-8")
        cls.workbench_blueprint_cpp = WORKBENCH_BLUEPRINT_CPP.read_text(
            encoding="utf-8"
        )

    def test_typed_balance_api_is_manual_only_and_stages_on_a_copy(self) -> None:
        for name in (
            "Insert_ValtanManualStageAfter",
            "Remove_ValtanManualStage",
            "Move_ValtanManualStage",
        ):
            self.assertIn(name, self.balance_h)
            body = function_body(
                self.balance_cpp, f"bool Client::CBalanceTool::{name}("
            )
            self.assertIn("Require_ValtanAuthoringAdmission", body)
            self.assertIn("bManualServerAudition", body)
            self.assertIn("VALTAN_PATTERN_TREE_VIEW staged = m_valtanPatternTree", body)
            self.assertLess(
                body.index("VALTAN_PATTERN_TREE_VIEW staged = m_valtanPatternTree"),
                body.index("m_valtanPatternTree = std::move(staged)"),
            )
        insert = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::Insert_ValtanManualStageAfter(",
        )
        self.assertIn("IsValtanStableAuthoringId(stageId)", insert)
        self.assertIn("IsValtanStableAuthoringId(actionId)", insert)
        self.assertIn("IsValtanManualStageRole(stageRole)", insert)
        self.assertIn("current->Stages.size() >= 64u", insert)

    def test_wait_reuses_active_none_contract(self) -> None:
        constructor = function_body(
            self.balance_cpp, "VALTAN_STAGE_VIEW BuildValtanManualStage("
        )
        self.assertIn(
            'stage.strStageKind = "WAIT" == stageRole ? "ACTIVE" : stageRole',
            constructor,
        )
        self.assertIn('stage.strAnimationEndPolicy = "NONE"', constructor)
        self.assertIn("stage.bSuppressAnimation = true", constructor)
        self.assertIn('stage.strHitShape = "NONE"', constructor)
        self.assertIn(
            'AddValtanClosedFlagActions(stage, "boss.flag.groggy")',
            constructor,
        )
        self.assertNotIn('stage.strStageKind = "WAIT";', constructor)

    def test_patch_replays_add_role_counter_then_remove_move_and_fields(self) -> None:
        emitter = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::BuildValtanDraftPatch(",
        )
        for operation in (
            "INSERT_MANUAL_STAGE_AFTER",
            "REMOVE_MANUAL_STAGE",
            "MOVE_MANUAL_STAGE",
        ):
            self.assertIn(operation, self.balance_cpp)
        phases = (
            "counterDisableOperations.begin(), counterDisableOperations.end()",
            "manualStagePreCounterTopologyOperations.begin()",
            "stageRoleOperations.begin(), stageRoleOperations.end()",
            "counterEnableOperations.begin(), counterEnableOperations.end()",
            "manualStagePostCounterTopologyOperations.begin()",
            "operations.begin(), operations.end()",
        )
        positions = [emitter.index(token) for token in phases]
        self.assertEqual(sorted(positions), positions)
        self.assertIn("BuildValtanManualStageTopologyPatch(", emitter)
        self.assertIn("loadedTopology = &topologyBaseline", emitter)
        self.assertIn("Canonical Pattern Stage topology is read-only", emitter)

    def test_counter_disable_is_replayed_into_baseline_before_target_remove(self) -> None:
        emitter = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::BuildValtanDraftPatch(",
        )
        normalize = emitter.index(
            "appendTopology(counterDisableOperations, operation)"
        )
        topology = emitter.index("BuildValtanManualStageTopologyPatch(")
        self.assertLess(normalize, topology)
        self.assertIn(
            'RemoveValtanFlagActions(\n\t\t\t\t\t\t*normalizedStage, '
            '"boss.flag.counterable")',
            emitter,
        )
        self.assertIn('return "COUNTER_HIT" == branch.strOutcome', emitter)
        self.assertIn(
            "m_valtanPatternTree, pattern,\n\t\t\t\t\t\tcounterNormalizedLoaded",
            emitter,
        )

        retarget = emitter.index("const bool targetChanged")
        normalized_edge = emitter.index(
            "branch->strNextActionId = currentCounter.successActionId", retarget
        )
        topology = emitter.index("BuildValtanManualStageTopologyPatch(", retarget)
        self.assertLess(retarget, normalized_edge)
        self.assertLess(normalized_edge, topology)

        stage_setter = function_body(
            self.balance_cpp, "bool Client::CBalanceTool::Set_ValtanStageDraft("
        )
        self.assertIn(
            'RemoveValtanFlagActions(*stage, "boss.flag.groggy")',
            stage_setter,
        )
        self.assertIn(
            'AddValtanClosedFlagActions(*stage, "boss.flag.groggy")',
            stage_setter,
        )
        self.assertLess(
            stage_setter.index("stage->Actions = candidate.actions"),
            stage_setter.index(
                'RemoveValtanFlagActions(*stage, "boss.flag.groggy")'
            ),
        )

        topology_builder = function_body(
            self.balance_cpp, "bool BuildValtanManualStageTopologyPatch("
        )
        self.assertIn(
            "CanRemoveValtanManualStage(\n\t\t\t\t\tdependencyTree, current",
            topology_builder,
        )

    def test_remove_rejects_dangling_typed_dependencies(self) -> None:
        guard = function_body(
            self.balance_cpp, "bool CanRemoveValtanManualStage("
        )
        for token in (
            "branch.strNextActionId",
            "pattern.ServerMotion",
            "pattern.Reactions",
            "pattern.WorldEventTriggerRefs",
            "tree.CounterReactionLayers",
            "tree.IndependentEffects",
            "stage.Actions.empty()",
            "stage.Branches.empty()",
            "stage.CounterProxy.has_value()",
            "stage.Motion.has_value()",
            "stage.ProductCues.empty()",
            "stage.CameraInvocations.empty()",
        ):
            self.assertIn(token, guard)
        self.assertIn("IsValtanManualStageRole(stage.strSequenceRole)", guard)
        self.assertIn("remainingStageCount", guard)

        removal = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::Remove_ValtanManualStage(",
        )
        self.assertIn("m_loadedValtanPatternTree", removal)
        self.assertIn('"SOURCE_REVIEWED_DELTA"', removal)
        self.assertIn(
            "Save + Validate + Publish the Collider removal first",
            removal,
        )
        self.assertLess(
            removal.index("savedRemovalProvenance"),
            removal.index("VALTAN_PATTERN_TREE_VIEW staged = m_valtanPatternTree"),
        )

    def test_cpp_topology_gate_matches_the_linear_timeout_contract(self) -> None:
        gate = function_body(
            self.balance_cpp, "bool IsValtanManualStageTopologyLinear("
        )
        self.assertIn("pattern.strEntryActionId", gate)
        self.assertIn('"TIMEOUT" == branch.strOutcome', gate)
        self.assertIn("Timeout->strNextActionId != expectedNext", gate)
        for operation in (
            "Insert_ValtanManualStageAfter",
            "Remove_ValtanManualStage",
            "Move_ValtanManualStage",
        ):
            body = function_body(self.balance_cpp, f"bool Client::CBalanceTool::{operation}(")
            self.assertIn("IsValtanManualStageTopologyLinear", body)

        forward = function_body(
            self.balance_cpp, "bool IsValtanCounterTopologyFiniteForward("
        )
        self.assertIn("ReadValtanCounterWindow(pattern, source, counter, status)", forward)
        self.assertIn("sourceIndex", forward)
        self.assertIn("counter.successStageId", forward)

        move = function_body(
            self.balance_cpp, "bool Client::CBalanceTool::Move_ValtanManualStage("
        )
        move_gate = move.index("IsValtanCounterTopologyFiniteForward")
        self.assertLess(move_gate, move.index("m_valtanPatternTree = std::move(staged)"))

        counter = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::Set_ValtanCounterWindowDraft(",
        )
        counter_gate = counter.index("IsValtanCounterTopologyFiniteForward")
        self.assertLess(counter_gate, counter.index("*currentPattern = std::move(*stagedPattern)"))

    def test_unchanged_complex_manual_graph_bypasses_linear_topology_rewrite(self) -> None:
        builder = function_body(
            self.balance_cpp, "bool BuildValtanManualStageTopologyPatch("
        )
        for token in (
            "const bool stableTopology",
            "current.Stages.size() == loaded.Stages.size()",
            "currentStage.strStageId == loadedStage.strStageId",
            "currentStage.strActionId == loadedStage.strActionId",
            "if (stableTopology)",
        ):
            self.assertIn(token, builder)
        stable = builder.index("if (stableTopology)")
        linear = builder.index("IsValtanManualStageTopologyLinear")
        self.assertLess(stable, linear)
        self.assertIn("return true;", builder[stable:linear])

    def test_blueprint_owns_insert_move_remove_and_sound_preflight(self) -> None:
        topology = function_body(
            self.workbench_blueprint_cpp,
            "Render_BossPatternStageTopologyControls(",
        )
        for label in (
            '"ACTIVE##BossPatternAdd"',
            '"WINDUP##BossPatternAdd"',
            '"GROGGY##BossPatternAdd"',
            '"WAIT / GAP##BossPatternAdd"',
            '"Move Earlier##BossPattern"',
            '"Move Later##BossPattern"',
            '"Delete Stage##BossPattern"',
        ):
            self.assertIn(label, topology)
        self.assertIn("BuildNextBossPatternStageIdentity", topology)
        self.assertIn("Can_Edit_ValtanManualStageTopology", topology)
        self.assertIn("!bTopologyEditable", topology)
        self.assertIn("Topology is read-only:", topology)
        self.assertIn("Insert_ValtanManualStageAfter", topology)
        self.assertIn("Move_ValtanManualStage", topology)
        self.assertIn("Remove_ValtanManualStage", topology)
        self.assertLess(
            topology.index("Validate_ManualStageTopologySoundDependencies("),
            topology.index("Remove_ValtanManualStage("),
        )
        details = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_GameplayStageDetails(",
        )
        self.assertIn("Open Boss Pattern Structure", details)
        self.assertNotIn("Insert_ValtanManualStageAfter", details)
        self.assertNotIn("Remove_ValtanManualStage", details)
        sound = function_body(
            self.workbench_cpp,
            "Validate_ManualStageTopologySoundDependencies(",
        )
        self.assertIn(
            "Validate_ValtanCompositionPatternSoundGraphDependencies(", sound
        )
        self.assertIn("Get_ValtanPatternDraft(", sound)

    def test_new_active_none_stage_can_receive_sequence_but_wait_stays_blank(self) -> None:
        draft = function_body(self.balance_cpp, "PATTERN_STAGE_EDIT BuildValtanStageDraft(")
        self.assertIn("pattern.bManualServerAudition ||", draft)
        self.assertIn('const bool isWaitStage = "WAIT" == stage.strSequenceRole', draft)
        self.assertIn("pattern.bManualServerAudition && !isWaitStage", draft)
        self.assertIn("draft.hitEditable = !isWaitStage", draft)
        self.assertIn("draft.animationEditable = !isWaitStage", draft)
        setter = function_body(
            self.balance_cpp, "bool Client::CBalanceTool::Set_ValtanStageDraft("
        )
        self.assertIn("animationClockChanged", setter)
        self.assertIn("WAIT Stage owns only its Server clock", setter)
        self.assertIn("stage->bSuppressAnimation = false", setter)
        details = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_GameplayStageDetails(",
        )
        self.assertIn('const bool_t bWaitStage = "WAIT" == Stage.strSequenceRole', details)
        self.assertIn('"WINDUP" == Stage.strStageKind', details)
        self.assertIn("ImGui::BeginDisabled(!bCounterSourceEditable)", details)
        self.assertIn("iCandidate = iCurrentStageIndex + 1u", details)
        self.assertIn("WAIT is a clock-only gap", details)
        sequence = function_body(
            self.workbench_cpp,
            "bool_t Client::CActionCompositionWorkbench::Apply_SelectedSequenceToStage(",
        )
        self.assertIn("Replace Stage Slots", self.workbench_cpp)
        self.assertIn("Draft.animationSlots = std::move(Slots)", sequence)
        self.assertIn('"WAIT" == Stage.strSequenceRole', sequence)
        self.assertIn("must remain Animation NONE", sequence)
        self.assertIn('"EXACT" : "HOLD_LAST_POSE"', sequence)
        self.assertIn("FitCompositionSequenceCutsToStage(", sequence)
        self.assertIn("while (iRemainingMs > NativeDurationsMs[iClip])", sequence)
        self.assertIn("Slot.repeatUntilStageEnd = false", sequence)
        clock = function_body(self.workbench_cpp, "bool_t ApplyStageClockPolicy(")
        self.assertIn("Draft.animationSlots.empty()", clock)
        self.assertIn('"NONE" == Draft.animationEndPolicy', clock)

    def test_draft_only_stage_selection_survives_the_next_render_frame(self) -> None:
        normalize = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Normalize_Selection()",
        )
        self.assertNotIn("nullptr == Find_SelectedStage(pPattern)", normalize)
        self.assertIn("m_strSelectedStageId.empty()", normalize)

        render = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render()",
        )
        effective = render.index(
            "const VALTAN_PATTERN_VIEW* const pPattern = bEffectivePatternReady"
        )
        effective_normalize = render.index(
            "nullptr == Find_SelectedStage(pPattern)", effective
        )
        stage_lookup = render.index(
            "const VALTAN_STAGE_VIEW* const pStage = Find_SelectedStage(pPattern)",
            effective_normalize,
        )
        self.assertLess(effective, effective_normalize)
        self.assertLess(effective_normalize, stage_lookup)
        self.assertIn("Render_PatternsWindow(", render)

        patterns_window = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_PatternsWindow(",
        )
        self.assertIn("Render_Browser(pPattern)", patterns_window)

        browser = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_Browser(",
        )
        self.assertIn("pEffectiveSelectedPattern", browser)
        self.assertIn("pDisplayPattern->Stages", browser)
        self.assertIn("Select_Stage(*pDisplayPattern, Stage)", browser)

    def test_boss_graph_generation_belongs_to_the_immutable_pattern_view(self) -> None:
        render = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render()",
        )
        generation = render.index(
            "const std::uint64_t iPatternViewDraftGeneration"
        )
        sequencer = render.index("Render_SequencerWindow(", generation)
        graph = render.index("Render_BossPatternWindow(", sequencer)
        self.assertLess(generation, sequencer)
        self.assertLess(sequencer, graph)
        self.assertIn(
            "m_iEffectivePatternCacheDraftGeneration : 0u", render[generation:sequencer]
        )
        self.assertIn("pPattern, iPatternViewDraftGeneration", render[graph:])

        window = function_body(
            self.workbench_blueprint_cpp,
            "void Client::CActionCompositionWorkbench::Render_BossPatternWindow(",
        )
        self.assertNotIn("Get_ValtanDraftGeneration", window)
        self.assertIn(
            "m_iBossPatternGraphAttemptDraftGeneration ==\n\t\t\tiPatternViewDraftGeneration",
            window,
        )
        self.assertIn(
            "*pPattern, iPatternViewDraftGeneration", window
        )

    def test_topology_mutation_resets_preview_route_and_rejected_graph_is_not_hit(self) -> None:
        topology = function_body(
            self.workbench_blueprint_cpp,
            "Render_BossPatternStageTopologyControls(",
        )
        changed = topology.index("if (bChanged)")
        self.assertIn(
            "m_BossPatternOutcomeOverrides.clear()", topology[changed:]
        )
        self.assertIn("++m_iBossPatternRouteGeneration", topology[changed:])

        window = function_body(
            self.workbench_blueprint_cpp,
            "void Client::CActionCompositionWorkbench::Render_BossPatternWindow(",
        )
        self.assertIn('ImGui::SmallButton("Reset Route")', window)
        self.assertIn("bool_t bCanInteractSnapshot", window)
        self.assertIn(
            "if (bCanInteractSnapshot && bCanvasHovered", window
        )
        self.assertIn("MatchesBossPatternEdgeSource(", window)
        self.assertIn("Edge.iSourceBranchIndex", self.workbench_blueprint_cpp)


if __name__ == "__main__":
    unittest.main(verbosity=2)
