from __future__ import annotations

import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
BALANCE_H = ROOT / "Client" / "Public" / "BalanceTool.h"
BALANCE_CPP = ROOT / "Client" / "Private" / "BalanceTool.cpp"
PATTERN_TREE_CPP = ROOT / "Client" / "Private" / "ValtanPatternTree.cpp"
WORKBENCH_CPP = ROOT / "Client" / "Private" / "ActionCompositionWorkbench.cpp"


def _slice(source: str, begin: str, end: str) -> str:
    start = source.index(begin)
    stop = source.index(end, start)
    return source[start:stop]


class ValtanClientCounterBranchSourceContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.header = BALANCE_H.read_text(encoding="utf-8")
        cls.balance = BALANCE_CPP.read_text(encoding="utf-8")
        cls.pattern_tree = PATTERN_TREE_CPP.read_text(encoding="utf-8")
        cls.workbench = WORKBENCH_CPP.read_text(encoding="utf-8")

    def test_typed_draft_preserves_success_and_timeout_stable_identity(self) -> None:
        draft = _slice(
            self.header,
            "struct VALTAN_COUNTER_WINDOW_EDIT final",
            "struct VALTAN_COUNTER_PROXY_EDIT final",
        )
        for field in (
            "successStageId",
            "successActionId",
            "timeoutStageId",
            "timeoutActionId",
        ):
            self.assertIn(field, draft)

        reader = _slice(
            self.balance,
            "bool ReadValtanCounterWindow(",
            "bool IsValtanCounterTopologyFiniteForward(",
        )
        self.assertIn("counterBranches", reader)
        self.assertIn("timeoutBranches", reader)
        self.assertIn("1u != timeoutBranches.size()", reader)
        self.assertIn("output.timeoutStageId", reader)
        self.assertIn("output.timeoutActionId", reader)

    def test_success_and_timeout_are_forward_typed_counter_edges(self) -> None:
        helper = _slice(
            self.balance,
            "bool IsValtanCounterSuccessStageKind(",
            "int ValtanFlagContractState(",
        )
        for kind in ("WINDUP", "GROGGY", "RECOVERY"):
            self.assertIn(f'"{kind}"', helper)

        forward = _slice(
            self.balance,
            "bool IsValtanCounterTopologyFiniteForward(",
            "void RemoveValtanFlagActions(",
        )
        self.assertIn("counter.successStageId", forward)
        self.assertIn("counter.timeoutStageId", forward)
        self.assertIn("target - pattern.Stages.begin()", forward)

        setter = _slice(
            self.balance,
            "bool Client::CBalanceTool::Set_ValtanCounterWindowDraft(",
            "bool Client::CBalanceTool::Get_ValtanCounterProxyDraft(",
        )
        self.assertIn("counter.timeoutStageId", setter)
        self.assertIn("counter.timeoutActionId", setter)
        self.assertIn("source Stage owns duplicate TIMEOUT branches", setter)
        self.assertIn('timeoutBranch.strOutcome = "TIMEOUT"', setter)
        self.assertIn('"GROGGY" == target->strStageKind && 0 == groggyState', setter)
        self.assertNotIn("same-pattern GROGGY stage", setter)

    def test_manual_topology_preserves_counter_owned_timeout(self) -> None:
        topology = _slice(
            self.balance,
            "bool IsValtanManualStageTopologyLinear(",
            "bool BuildValtanManualStageTopologyPatch(",
        )
        self.assertGreaterEqual(topology.count("counterOwnsTimeout"), 4)
        self.assertIn("timeoutTarget - pattern.Stages.begin()", topology)
        self.assertIn('"TIMEOUT" == branch.strOutcome && !counterOwnsTimeout', topology)

    def test_every_patch_emission_carries_timeout_stable_identity(self) -> None:
        patch_builder = self.balance[self.balance.index("BuildValtanDraftPatch(") :]
        self.assertEqual(
            4,
            patch_builder.count('\\"op\\": \\"SET_STAGE_COUNTER_WINDOW\\"'),
        )
        self.assertEqual(4, patch_builder.count('\\"timeoutStageId\\"'))
        self.assertEqual(4, patch_builder.count('\\"timeoutActionId\\"'))
        self.assertIn(
            "currentCounter.timeoutStageId != loadedCounter.timeoutStageId",
            patch_builder,
        )
        self.assertIn(
            "timeout->strNextActionId = currentCounter.timeoutActionId",
            patch_builder,
        )

    def test_split_loader_uses_conditional_groggy_and_forward_timeout(self) -> None:
        validator = _slice(
            self.pattern_tree,
            "bool_t Validate_SplitCounterBranchContract(",
            "bool_t Read_RequiredHitOffsets(",
        )
        self.assertIn("iCounterCount", validator)
        self.assertIn("iTimeoutCount", validator)
        self.assertIn("iTargetIndex <= iStageIndex", validator)
        self.assertIn("iTimeoutTargetIndex <= iStageIndex", validator)
        for kind in ("WINDUP", "GROGGY", "RECOVERY"):
            self.assertIn(f'"{kind}"', validator)
        self.assertIn('"GROGGY" == strTargetKind && 1 != iTargetGroggyState', validator)
        self.assertIn('"GROGGY" != strTargetKind && 0 != iTargetGroggyState', validator)
        self.assertNotIn("same-pattern GROGGY target", validator)

    def test_workbench_edits_both_counter_branch_targets(self) -> None:
        counter_detail = _slice(
            self.workbench,
            'ImGui::SeparatorText("Counter Hurt Proxy (Player -> Boss)")',
            'if (bChanged && !bWarpRushAppliedThisFrame)',
        )
        self.assertIn("SuccessTargets", counter_detail)
        self.assertIn("TimeoutTargets", counter_detail)
        self.assertIn('"Counter Success Stage"', counter_detail)
        self.assertIn('"Counter Timeout Stage"', counter_detail)
        self.assertNotIn('"Counter Success Groggy"', counter_detail)


if __name__ == "__main__":
    unittest.main()
