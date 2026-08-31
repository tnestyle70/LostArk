#!/usr/bin/env python3
"""Static UI/process boundary for Anim Workbench Create New Pattern."""

from __future__ import annotations

from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[2]


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8")


def function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for cursor in range(opening, len(source)):
        token = source[cursor]
        if token == "{":
            depth += 1
        elif token == "}":
            depth -= 1
            if depth == 0:
                return source[opening : cursor + 1]
    raise AssertionError(f"unterminated function: {signature}")


class ValtanAnimationPatternCreateWorkbenchContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.animation_h = read("Client/Public/Animation_Tool.h")
        cls.animation_cpp = read("Client/Private/Animation_Tool.cpp")
        cls.boss_h = read("Client/Public/BossTool.h")
        cls.boss_cpp = read("Client/Private/BossTool.cpp")
        cls.backend = read(
            "Tools/ValtanPipeline/promote_valtan_animation_chains.py"
        )

    def test_ui_exposes_current_saved_validate_and_explicit_apply(self) -> None:
        panel = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanPatternCreatePanel()",
        )
        for token in (
            'ImGui::SeparatorText("Create New Pattern")',
            '"Current assembled chain"',
            '"Saved intake chain"',
            '"Stable patternId"',
            '"Display name"',
            '"Authoring phase"',
            '"Target policy"',
            '"Aim policy"',
            '"Validate Create Request"',
            '"Apply Create Pattern"',
            "m_strValtanPatternCreateValidatedRequestSha256",
        ):
            self.assertIn(token, panel)

    def test_request_builder_uses_only_strict_backend_fields(self) -> None:
        builder = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Build_ValtanPatternCreateRequest(",
        )
        for token in (
            "lostark.valtan-animation-pattern-create-request",
            '"expectedSourceSha256"',
            '"patternId"',
            '"displayName"',
            '"authoringPhase"',
            '"targetPolicy"',
            '"aimPolicy"',
            '"SAVED_INTAKE_CHAIN"',
            '"CURRENT_CHAIN"',
            '"sourceActionId"',
            '"sourceSequenceIndex"',
            '"NATIVE_CLIP_LENGTHS"',
            '"PROJECT_AUTHORED"',
        ):
            self.assertIn(token, builder)
        for forbidden in (
            '"hit"',
            '"motion"',
            '"events"',
            '"branches"',
            '"reactions"',
        ):
            self.assertNotIn(forbidden, builder)

    def test_selected_sequence_identity_survives_create_request(self) -> None:
        stage = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Stage_ValtanCompositionIntakeSequence(",
        )
        builder = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Build_ValtanPatternCreateRequest(",
        )
        self.assertIn(
            "m_iValtanPatternCreateSourceActionId = iSkillId", stage
        )
        self.assertIn(
            "m_iValtanPatternCreateSourceSequenceIndex = iSequenceIndex", stage
        )
        self.assertIn("m_iValtanPatternCreateSourceActionId", builder)
        self.assertIn("m_iValtanPatternCreateSourceSequenceIndex", builder)
        self.assertNotIn('"sourceSequenceIndex": 1', self.backend)

    def test_process_boundary_uses_request_file_without_a_shell(self) -> None:
        start = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Start_ValtanPatternCreateCommand(",
        )
        for token in (
            "Resolve_PythonExecutable",
            "CreateProcessW",
            "Python.c_str()",
            "--request-file",
            "--mode",
            "CREATE_NO_WINDOW",
            "m_ValtanPatternCreateRequestPath",
            "m_ValtanPatternCreateDiagnosticPath",
        ):
            self.assertIn(token, start)
        for forbidden in ("system(", "_popen(", "popen(", "ShellExecute"):
            self.assertNotIn(forbidden, start)

    def test_success_reloads_joined_master_and_typed_boss_inventory(self) -> None:
        poll = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Poll_ValtanPatternCreateCommand()",
        )
        self.assertIn("Parse_ValtanPatternCreateResult", poll)
        self.assertIn("m_pBalanceTool->Reload_ValtanSource", poll)
        self.assertNotIn("m_pBalanceTool->Save_ValtanProduct", poll)
        self.assertIn("Reload_ValtanPatternMaster()", poll)
        self.assertIn("m_pBossTool->Reload_CanonicalGraph", poll)
        self.assertIn("bReloadClosureAdmitted", poll)
        self.assertIn("m_bValtanCompositionPatternCreatedPending = true", poll)
        self.assertIn("m_strValtanPatternCreateValidatedRequestSha256", poll)
        self.assertLess(
            poll.index("m_pBalanceTool->Reload_ValtanSource"),
            poll.index("Reload_ValtanPatternMaster()"),
        )
        self.assertLess(
            poll.index("Reload_ValtanPatternMaster()"),
            poll.index("m_pBossTool->Reload_CanonicalGraph"),
        )

        consume = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Consume_ValtanCompositionPatternCreated(",
        )
        self.assertIn("m_bValtanCompositionPatternCreatedPending = false", consume)
        self.assertIn("m_strValtanCompositionPatternCreatedId.clear()", consume)

        self.assertIn(
            "bool_t Reload_CanonicalGraph(std::string& strOutStatus);",
            self.boss_h,
        )
        reload_graph = function_body(
            self.boss_cpp,
            "bool_t Client::CBossTool::Reload_CanonicalGraph(",
        )
        self.assertIn("Reload_Graph()", reload_graph)

    def test_apply_blocks_dirty_typed_owners_before_process_mutation(self) -> None:
        start = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Start_ValtanPatternCreateCommand(",
        )
        process_start = start.index("CreateProcessW")
        for token in (
            "m_pBalanceTool->Is_ValtanDraftDirty()",
            "Is_ValtanDocumentDirty()",
        ):
            self.assertIn(token, start)
            self.assertLess(start.index(token), process_start)

    def test_backend_result_and_safe_defaults_remain_authoritative(self) -> None:
        for token in (
            'CREATE_RESULT_SCHEMA = "lostark.valtan-animation-pattern-create-result"',
            '"admissionState": "MANUAL_SERVER_AUDITION"',
            '"selectionMode": "AUDITION_ONLY"',
            '"hit": {"shape": {"kind": "NONE"}}',
            '"motion": None',
            '"events": []',
            '"branches": []',
        ):
            self.assertIn(token, self.backend)


if __name__ == "__main__":
    unittest.main()
