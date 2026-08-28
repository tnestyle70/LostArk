#!/usr/bin/env python3

from __future__ import annotations

import json
from pathlib import Path
import unittest


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
CPP_PATH = REPOSITORY_ROOT / "Client/Private/Animation_Tool.cpp"
HEADER_PATH = REPOSITORY_ROOT / "Client/Public/Animation_Tool.h"
VALTAN_CPP_PATH = REPOSITORY_ROOT / "Client/Private/Valtan.cpp"
MASTER_PATH = REPOSITORY_ROOT / "Data/Valtan/Valtan.pattern.json"
PRESENTATION_PATH = REPOSITORY_ROOT / "Data/Valtan/Valtan.presentation.json"
GAMEPLAY_PATH = REPOSITORY_ROOT / "Data/Valtan/Valtan.gameplay.json"
PATTERN_BINDINGS_PATH = (
    REPOSITORY_ROOT
    / "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
)
PROMOTION_MANIFEST_PATH = (
    REPOSITORY_ROOT / "Data/Valtan/Valtan.animation-chain-promotions.json"
)

EXPECTED_PATTERN_ORDER = (
    "VALTAN_WHIRLWIND",
    "VALTAN_DASH_CHARGE",
    "VALTAN_FOUR_SLASH",
    "VALTAN_FIST_IN_OUT",
    "VALTAN_HIGH_JUMP",
    "VALTAN_FLOOR_WIPE_130",
    "VALTAN_ARENA_BREAK_109",
)

def function_slice(text: str, signature: str, next_signature: str) -> str:
    start = text.index(signature)
    end = text.index(next_signature, start + len(signature))
    return text[start:end]


class AnimationToolValtanPatternMasterContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.cpp = CPP_PATH.read_text(encoding="utf-8")
        cls.header = HEADER_PATH.read_text(encoding="utf-8")
        cls.valtan_cpp = VALTAN_CPP_PATH.read_text(encoding="utf-8")
        cls.master = json.loads(MASTER_PATH.read_text(encoding="utf-8"))
        cls.presentation = json.loads(PRESENTATION_PATH.read_text(encoding="utf-8"))
        cls.gameplay = json.loads(GAMEPLAY_PATH.read_text(encoding="utf-8"))
        cls.pattern_bindings = json.loads(
            PATTERN_BINDINGS_PATH.read_text(encoding="utf-8")
        )
        cls.promotion_manifest = json.loads(
            PROMOTION_MANIFEST_PATH.read_text(encoding="utf-8")
        )

    def test_master_contains_the_seven_animation_tool_patterns(self) -> None:
        self.assertEqual("lostark.valtan-pattern-master", self.master["schema"])
        self.assertEqual(1, self.master["formatVersion"])
        self.assertEqual(
            list(EXPECTED_PATTERN_ORDER),
            [pattern["patternId"] for pattern in self.master["patterns"]],
        )

    def test_animation_tool_loads_the_shared_typed_projection(self) -> None:
        combined = self.header + "\n" + self.cpp
        for token in (
            '#include "ValtanPatternTree.h"',
            "VALTAN_PATTERN_TREE_VIEW m_ValtanPatternMasterView;",
            "CValtanPatternTree::Load(Staged, Status)",
            "bAuthoringMasterManaged",
            "expected the 7 baseline managed patterns",
            '"Valtan Pattern Master (Authoritative)"',
            "Data/Valtan/Valtan.gameplay.json + Valtan.presentation.json",
        ):
            self.assertIn(token, combined)

    def test_live_pattern_inventory_is_a_dynamic_stable_id_join(self) -> None:
        gameplay_ids = [pattern["patternId"] for pattern in self.gameplay["patterns"]]
        presentation_ids = [
            pattern["patternId"] for pattern in self.presentation["patterns"]
        ]
        self.assertTrue(gameplay_ids)
        self.assertEqual(len(gameplay_ids), len(set(gameplay_ids)))
        self.assertEqual(len(presentation_ids), len(set(presentation_ids)))
        self.assertEqual(set(gameplay_ids), set(presentation_ids))

    def test_every_product_pattern_and_promoted_audition_is_locally_selectable(self) -> None:
        gameplay_ids = {pattern["patternId"] for pattern in self.gameplay["patterns"]}
        presentation_ids = {
            pattern["patternId"] for pattern in self.presentation["patterns"]
        }
        self.assertEqual(gameplay_ids, presentation_ids)
        self.assertTrue(gameplay_ids)
        self.assertEqual(len(gameplay_ids), len(self.gameplay["patterns"]))
        self.assertEqual(len(presentation_ids), len(self.presentation["patterns"]))

        decision_rows = self.gameplay["decisionModel"]["manualAuditions"]
        manual_rows = [
            row for row in decision_rows
            if row["admissionState"] == "MANUAL_SERVER_AUDITION"
        ]
        derived_rows = [
            row for row in decision_rows
            if row["admissionState"] == "DERIVED_SERVER_PATTERN"
        ]
        self.assertEqual(len(decision_rows), len(manual_rows) + len(derived_rows))
        for row in decision_rows:
            self.assertRegex(row["patternId"], r"^[A-Za-z0-9_.-]{1,128}$")
            self.assertRegex(row["sourceChainId"], r"^[A-Za-z0-9_.-]{1,128}$")
            self.assertIs(type(row["authoringPhase"]), int)
            self.assertIn(row["authoringPhase"], (1, 2, 3))
        decision_ids = [row["patternId"] for row in decision_rows]
        decision_chains = [row["sourceChainId"] for row in decision_rows]
        self.assertEqual(len(decision_ids), len(set(decision_ids)))
        self.assertEqual(len(decision_chains), len(set(decision_chains)))
        self.assertTrue(set(decision_ids).issubset(gameplay_ids))
        self.assertTrue(set(decision_ids).issubset(presentation_ids))

        manifest_rows = self.promotion_manifest["patterns"]
        lineage = lambda row: (
            row["patternId"], row["sourceChainId"],
            row["authoringPhase"], row["admissionState"],
        )
        self.assertEqual(
            [lineage(row) for row in manual_rows],
            [lineage(row) for row in manifest_rows],
        )
        manifest_ids = [row["patternId"] for row in manifest_rows]
        manifest_chains = [row["sourceChainId"] for row in manifest_rows]
        self.assertEqual(len(manifest_ids), len(set(manifest_ids)))
        self.assertEqual(len(manifest_chains), len(set(manifest_chains)))
        self.assertFalse(
            {row["patternId"] for row in derived_rows} & set(manifest_ids)
        )
        self.assertFalse(
            {row["sourceChainId"] for row in derived_rows} & set(manifest_chains)
        )
        gameplay_names = {
            row["patternId"]: row["displayName"]
            for row in self.gameplay["patterns"]
        }
        for row in manifest_rows:
            self.assertEqual(gameplay_names[row["patternId"]], row["displayName"])

        source_path = REPOSITORY_ROOT / self.promotion_manifest["sourceDocument"]["path"]
        debug_document = json.loads(source_path.read_text(encoding="utf-8"))
        debug_chains = [row["chainId"] for row in debug_document["chains"]]
        intake_chains = [
            row["sourceChainId"]
            for row in self.promotion_manifest["animationIntakeOnly"]
        ]
        all_promoted_chains = set(manifest_chains) | set(intake_chains)
        self.assertEqual(all_promoted_chains, set(debug_chains))
        self.assertEqual(manifest_chains, [
            chain_id for chain_id in debug_chains if chain_id in set(manifest_chains)
        ])
        self.assertEqual(intake_chains, [
            chain_id for chain_id in debug_chains if chain_id in set(intake_chains)
        ])

        retired_id = "VALTAN_SEQUENCE_FRONT_BACK_FRONT"
        self.assertNotIn(retired_id, gameplay_ids)
        self.assertIn(retired_id, self.gameplay["retiredPatternIds"])
        for rows in (self.gameplay["patterns"], manifest_rows):
            four = next(row for row in rows if row["patternId"] == "VALTAN_SEQUENCE_FOUR")
            self.assertEqual("2페이즈 4방향 공격", four["displayName"])

        bound_actions = {
            binding["actionId"] for binding in self.pattern_bindings["bindings"]
        }
        product_actions = {
            stage["actionId"]
            for pattern in self.presentation["patterns"]
            for stage in pattern["stages"]
        }
        self.assertFalse(product_actions - bound_actions)

        collect = function_slice(
            self.cpp,
            "Client::CAnimation_Tool::Collect_ValtanPatternMasterPatterns() const",
            "bool_t Client::CAnimation_Tool::Reload_ValtanPatternMaster()",
        )
        build = function_slice(
            self.cpp,
            "bool_t Client::CAnimation_Tool::Build_ValtanPatternMasterTimeline(",
            "bool_t Client::CAnimation_Tool::Start_ValtanPatternMasterPreview(",
        )
        self.assertIn("AppendProductPresentation", collect)
        self.assertIn("!Pattern.Stages.empty()", collect)
        self.assertNotIn("Pattern.bAuthoringMasterManaged &&", collect)
        self.assertNotIn("!Pattern.bAuthoringMasterManaged", build)

    def test_recovered_floor_wipe_and_arena_break_sequences_are_exact(self) -> None:
        patterns = {
            row["patternId"]: row
            for row in self.presentation["patterns"]
        }
        floor = patterns["VALTAN_FLOOR_WIPE_130"]
        self.assertEqual(
            [
                "mesh_att_battle_5_02_loop",
                "mesh_att_battle_5_02_end",
                "mesh_att_battle_5_04",
                "mesh_att_battle_15_02",
                "mesh_att_battle_15_03",
                "mesh_att_battle_15_04",
            ],
            [
                occurrence["clip"]
                for stage in floor["stages"]
                for occurrence in stage["animation"]["occurrences"]
            ],
        )
        floor_stages = {
            stage["stageId"]: stage
            for stage in floor["stages"]
        }
        self.assertEqual(
            ("HOLD_LAST_POSE", [("mesh_att_battle_5_02_end", 534, False)]),
            (
                floor_stages["FIRST_SMASH"]["animation"]["endPolicy"],
                [
                    (
                        occurrence["clip"],
                        occurrence["playMs"],
                        occurrence["repeatUntilStageEnd"],
                    )
                    for occurrence in floor_stages["FIRST_SMASH"]["animation"]["occurrences"]
                ],
            ),
        )
        self.assertEqual(
            (
                "HOLD_LAST_POSE",
                [
                    ("mesh_att_battle_5_04", 500, False),
                    ("mesh_att_battle_15_02", 1000, False),
                ],
            ),
            (
                floor_stages["INTERVAL"]["animation"]["endPolicy"],
                [
                    (
                        occurrence["clip"],
                        occurrence["playMs"],
                        occurrence["repeatUntilStageEnd"],
                    )
                    for occurrence in floor_stages["INTERVAL"]["animation"]["occurrences"]
                ],
            ),
        )
        self.assertEqual(
            ("EXACT", [("mesh_att_battle_15_03", 500, False)]),
            (
                floor_stages["SECOND_SMASH"]["animation"]["endPolicy"],
                [
                    (
                        occurrence["clip"],
                        occurrence["playMs"],
                        occurrence["repeatUntilStageEnd"],
                    )
                    for occurrence in floor_stages["SECOND_SMASH"]["animation"]["occurrences"]
                ],
            ),
        )
        arena_break = patterns["VALTAN_ARENA_BREAK_109"]
        arena_gameplay = next(
            pattern
            for pattern in self.gameplay["patterns"]
            if pattern["patternId"] == "VALTAN_ARENA_BREAK_109"
        )
        self.assertEqual(
            {
                "kind": "LEAP_TO_ANCHOR",
                "anchorId": "anchor.valtan.arena-break-109.landing",
                "landingPosition": [156.03, 22.99751, -122.06],
                "apexHeight": 12.0,
                "travelStageId": "DROP",
                "takeoffStartMs": 0,
                "takeoffEndMs": 900,
                "travelStartMs": 0,
                "travelEndMs": 700,
            },
            arena_gameplay["serverMotion"],
        )
        self.assertEqual(
            [
                ("TAKEOFF", "valtan.mechanic.arena-break-109.takeoff", 900),
                ("DROP", "valtan.mechanic.arena-break-109.drop", 700),
                ("IMPACT", "valtan.mechanic.arena-break-109.impact", 400),
                (
                    "IMPACT_HOLD",
                    "valtan.mechanic.arena-break-109.impact-hold",
                    1100,
                ),
                (
                    "WIDE_REVEAL",
                    "valtan.mechanic.arena-break-109.wide-reveal",
                    2300,
                ),
                ("RECOVERY", "valtan.mechanic.arena-break-109.recovery", 870),
            ],
            [
                (stage["stageId"], stage["actionId"], stage["durationMs"])
                for stage in arena_gameplay["stages"]
            ],
        )
        self.assertEqual(
            [
                "mesh_att_battle_12_01",
                "mesh_att_battle_12_01",
                "mesh_att_battle_12_02",
                "mesh_att_battle_12_02",
                "mesh_att_battle_12_02",
                "mesh_att_battle_12_03",
                "mesh_att_battle_12_03",
                "mesh_evt1_att_battle_5_01_start",
                "mesh_evt1_att_battle_5_01_loop",
                "mesh_evt1_att_battle_5_01_end",
            ],
            [
                occurrence["clip"]
                for stage in arena_break["stages"]
                for occurrence in stage["animation"]["occurrences"]
            ],
        )
        wide_reveal = next(
            stage
            for stage in arena_break["stages"]
            if stage["stageId"] == "WIDE_REVEAL"
        )
        self.assertEqual("ROAR_SEQUENCE", wide_reveal["sequenceRole"])
        self.assertEqual(
            [
                "mesh_att_battle_12_03",
                "mesh_evt1_att_battle_5_01_start",
                "mesh_evt1_att_battle_5_01_loop",
            ],
            [
                occurrence["clip"]
                for occurrence in wide_reveal["animation"]["occurrences"]
            ],
        )

    def test_complete_timeline_uses_server_wall_and_source_clocks(self) -> None:
        build = function_slice(
            self.cpp,
            "bool_t Client::CAnimation_Tool::Build_ValtanPatternMasterTimeline(",
            "bool_t Client::CAnimation_Tool::Start_ValtanPatternMasterPreview(",
        )
        apply_pose = function_slice(
            self.cpp,
            "bool_t Client::CAnimation_Tool::Apply_ValtanPatternMasterPose(",
            "bool_t Client::CAnimation_Tool::Activate_ValtanPatternMasterItem(",
        )
        for token in (
            "Clip.iAuthoringWallMs",
            "Clip.iSourceStartMs",
            "Clip.iPlayMs",
            "Clip.fPlayRate",
            "Clip.bLoop",
            "iStageAnimationWallMs != Stage.iDurationMs",
            "Stage.iAuthoringRepeatCount",
            "iPlayableOccurrenceCount",
        ):
            self.assertIn(token, build)
        for token in (
            "m_ValtanPatternMasterBoss.lock()",
            "Boss->Apply_LocalPatternPresentationSample(",
            "ePatternAction",
            "Item.strStageKind",
            "Item.strActionId",
            "Item.iStageTimelineStartMs",
            "fStageWallSeconds",
            "Set_AnimPaused(true)",
        ):
            self.assertIn(token, apply_pose)

        shared_sampler = function_slice(
            self.valtan_cpp,
            "bool_t CValtan::Apply_PatternPresentationSample(",
            "bool_t CValtan::Apply_LocalPatternPresentationSample(",
        )
        for token in (
            "m_PatternClipByActionId.find",
            "Build_PatternTimeline(",
            "CActionPresentationTimeline::Resolve_Sample",
            "Requires_ClipOccurrenceTransition",
            "Start_Animation(",
            "Set_AnimationSpeed",
            "Set_AnimTrackPosition",
            "Play_Animation(0.f)",
        ):
            self.assertIn(token, shared_sampler)

        network_apply = function_slice(
            self.valtan_cpp,
            "bool_t CValtan::Apply_NetworkState(",
            "unique_ptr<CValtan> CValtan::Create(",
        )
        local_apply = function_slice(
            self.valtan_cpp,
            "bool_t CValtan::Apply_LocalPatternPresentationSample(",
            "void CValtan::Reset_LocalPatternPresentationSample()",
        )
        self.assertIn("Apply_PatternPresentationSample(", network_apply)
        self.assertIn("Apply_PatternPresentationSample(", local_apply)
        self.assertIn(
            "case LostArk::Shared::WORLD_ENTITY_ACTION::PATTERN_ACTIVE:",
            network_apply,
        )
        self.assertRegex(
            network_apply,
            r"const bool_t patternEdgeChanged = isPatternState &&\s*"
            r"\(patternIdChanged \|\| actionIdChanged \|\|\s*"
            r"iPatternSequence != m_iServerPatternSequence \|\|\s*"
            r"iPatternStageIndex != m_iServerPatternStageIndex \|\|\s*"
            r"iActionStartTick != m_iServerActionStartTick\);",
        )
        self.assertRegex(
            network_apply,
            r"const bool_t bAnimationEdgeChanged =\s*"
            r"m_iState != nextState \|\| patternEdgeChanged;",
        )
        self.assertRegex(
            network_apply,
            r"Apply_PatternPresentationSample\(\s*actionId,\s*\*pClip,\s*"
            r"fActionAgeSeconds,\s*bAnimationEdgeChanged,",
        )
        self.assertRegex(
            shared_sampler,
            r"if \(bAnimationEdgeChanged \|\| bClipOccurrenceTransition\)\s*"
            r"\{\s*if \(!m_pBodyModelCom->Start_Animation\(",
        )
        self.assertNotIn("CActionPresentationTimeline::Resolve_Sample", apply_pose)
        self.assertIn('"Play Arena Presentation Locally"', self.cpp)

    def test_stage_wall_conversion_matches_the_dash_server_timeline(self) -> None:
        gameplay_dash = next(
            pattern
            for pattern in self.gameplay["patterns"]
            if pattern["patternId"] == "VALTAN_DASH_CHARGE"
        )
        presentation_dash = next(
            pattern
            for pattern in self.presentation["patterns"]
            if pattern["patternId"] == "VALTAN_DASH_CHARGE"
        )
        gameplay_by_stage = {
            stage["stageId"]: stage for stage in gameplay_dash["stages"]
        }
        presentation_by_stage = {
            stage["stageId"]: stage for stage in presentation_dash["stages"]
        }

        normal_stage_ids = ("WINDUP", "CHARGE", "RECOVERY")
        timeline_ms = 0.0
        occurrence_starts: dict[tuple[str, int], float] = {}
        for stage_id in normal_stage_ids:
            stage_start_ms = timeline_ms
            animation = presentation_by_stage[stage_id]["animation"]
            for occurrence_index, occurrence in enumerate(animation["occurrences"]):
                occurrence_starts[(stage_id, occurrence_index)] = timeline_ms
                self.assertGreater(occurrence["playMs"], 0)
                timeline_ms += occurrence["playMs"] / occurrence["playRate"]
            self.assertAlmostEqual(
                gameplay_by_stage[stage_id]["durationMs"],
                timeline_ms - stage_start_ms,
                delta=0.1,
            )

        self.assertAlmostEqual(1200.0, occurrence_starts[("WINDUP", 2)])
        self.assertAlmostEqual(3650.0, occurrence_starts[("CHARGE", 0)])
        self.assertAlmostEqual(5150.0, occurrence_starts[("RECOVERY", 0)])
        self.assertAlmostEqual(6050.0, timeline_ms, delta=0.1)

        # Apply_ValtanPatternMasterPose converts occurrence-local wall time to
        # the same stage-age clock consumed by CValtan's Product sampler.
        third_windup_stage_age_seconds = (1200.0 - 0.0) * 0.001 + 0.604
        self.assertAlmostEqual(1.804, third_windup_stage_age_seconds)

    def test_arena_fallback_mapping_and_exact_preview_cleanup_are_explicit(self) -> None:
        mapper = function_slice(
            self.cpp,
            "bool_t Try_ResolveValtanArenaPatternAction(",
            "void Skip_Space(",
        )
        for token in (
            '"WINDUP" == strStageKind',
            '"ACTIVE" == strStageKind || "GROGGY" == strStageKind',
            '"RECOVERY" == strStageKind || "PART_BREAK" == strStageKind',
            "WORLD_ENTITY_ACTION::PATTERN_WINDUP",
            "WORLD_ENTITY_ACTION::PATTERN_ACTIVE",
            "WORLD_ENTITY_ACTION::PATTERN_RECOVERY",
        ):
            self.assertIn(token, mapper)
        self.assertEqual(
            {"WINDUP", "ACTIVE", "RECOVERY", "GROGGY", "PART_BREAK"},
            {
                stage["stageKind"]
                for pattern in self.gameplay["patterns"]
                for stage in pattern["stages"]
            },
        )

        fallback_resolver = function_slice(
            self.valtan_cpp,
            "const std::string* Resolve_ValtanPresentationClip(",
            "}\n\n#ifdef _DEBUG",
        )
        for token in (
            "presentationClips.patternWindup",
            "presentationClips.patternActive",
            "presentationClips.patternRecovery",
        ):
            self.assertIn(token, fallback_resolver)

        network_apply = function_slice(
            self.valtan_cpp,
            "bool_t CValtan::Apply_NetworkState(",
            "unique_ptr<CValtan> CValtan::Create(",
        )
        local_apply = function_slice(
            self.valtan_cpp,
            "bool_t CValtan::Apply_LocalPatternPresentationSample(",
            "void CValtan::Reset_LocalPatternPresentationSample()",
        )
        self.assertIn("Resolve_ValtanPresentationClip(*pActor, action)", network_apply)
        self.assertIn(
            "Resolve_ValtanPresentationClip(*pActor, patternAction)",
            local_apply,
        )

        update = function_slice(
            self.cpp,
            "void Client::CAnimation_Tool::Update(",
            "void Client::CAnimation_Tool::Render(",
        )
        reset = function_slice(
            self.cpp,
            "void Client::CAnimation_Tool::Reset_ValtanPatternMasterPreviewState(",
            "void Client::CAnimation_Tool::Update_ValtanPatternMasterHitAreaPreview()",
        )
        self.assertIn("m_ValtanPatternMasterBoss.lock()", update)
        self.assertIn("Stop_ValtanPatternMasterPreview(", update)
        self.assertNotIn("CAnimationTargetService::Resolve_Boss()", reset)
        self.assertIn("m_ValtanPatternMasterBoss.lock()", reset)
        self.assertIn("PreviewBoss->Reset_LocalPatternPresentationSample()", reset)
        self.assertIn("PreviewModel->Set_AnimPaused(false)", reset)

        local_reset = function_slice(
            self.valtan_cpp,
            "void CValtan::Reset_LocalPatternPresentationSample()",
            "void CValtan::Load_PatternEffectCues()",
        )
        self.assertIn("Set_AnimPaused(false)", local_reset)

    def test_dash_paths_come_from_the_admitted_branch_graph(self) -> None:
        build = function_slice(
            self.cpp,
            "bool_t Client::CAnimation_Tool::Build_ValtanPatternMasterTimeline(",
            "bool_t Client::CAnimation_Tool::Start_ValtanPatternMasterPreview(",
        )
        self.assertIn("CValtanPatternTree::Build_PreviewStagePath(", build)
        self.assertIn("VALTAN_PATTERN_PREVIEW_PATH", self.header)
        self.assertNotIn('{ "WINDUP", "CHARGE", "RECOVERY" }', build)
        self.assertIn('"Dash authoring path##ValtanPatternMaster"', self.cpp)

    def test_source_provenance_and_end_policy_are_visible(self) -> None:
        self.assertIn("pSelected->PresentationSources", self.cpp)
        self.assertIn("Source.iSourceActionId", self.cpp)
        self.assertIn("Source.iSequenceIndex", self.cpp)
        self.assertIn("Stage.strAnimationEndPolicy", self.cpp)
        policies = {
            stage["animation"]["endPolicy"]
            for pattern in self.master["patterns"]
            for stage in pattern["stages"]
        }
        self.assertEqual(
            {"EXACT", "HOLD_LAST_POSE", "LOOP_TO_STAGE_END"}, policies
        )

    def test_seek_and_one_active_preview_are_explicit(self) -> None:
        start_master = function_slice(
            self.cpp,
            "bool_t Client::CAnimation_Tool::Start_ValtanPatternMasterPreview(",
            "bool_t Client::CAnimation_Tool::Apply_ValtanPatternMasterPose(",
        )
        start_source = function_slice(
            self.cpp,
            "bool_t Client::CAnimation_Tool::Start_ValtanSequencePreview(",
            "bool_t Client::CAnimation_Tool::Start_ValtanPatternPreview(",
        )
        self.assertLess(
            start_master.index("Build_ValtanPatternMasterTimeline("),
            start_master.index("Reset_ValtanPatternPreviewState("),
            "master must stage and validate before replacing the current pose",
        )
        self.assertIn("Reset_ValtanPatternMasterPreviewState(", start_source)
        for token in (
            "Seek_ValtanPatternMasterPreview(",
            '"##ValtanPatternMasterTimeline"',
            "m_bValtanPatternMasterPaused",
            "Stop_ValtanPatternMasterPreview(",
            '"mesh_idle_battle_1"',
        ):
            self.assertIn(token, self.cpp)
        seek = function_slice(
            self.cpp,
            "bool_t Client::CAnimation_Tool::Seek_ValtanPatternMasterPreview(",
            "void Client::CAnimation_Tool::Advance_ValtanPatternMasterPreview(",
        )
        self.assertIn("std::nextafter(fLocalSeconds, 0.f)", seek)

    def test_historical_reference_is_retained_and_demoted(self) -> None:
        for token in (
            '"Secondary / Read-only Source Reference (1-67)"',
            "Valtan.patternpreview.json",
            "Valtan.clipseq",
            '"Valtan Source Reference (Read-only)"',
            "CValtanPatternPreviewDocument::Load(",
            "Start_ValtanSequencePreview(",
        ):
            self.assertIn(token, self.cpp)


if __name__ == "__main__":
    unittest.main()
