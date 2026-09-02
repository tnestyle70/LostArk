#!/usr/bin/env python3
"""Focused admission for typed Valtan Counter -> Groggy authoring."""

from __future__ import annotations

import copy
import pathlib
import sys
import tempfile
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import valtan_tuning_pipeline as pipeline


ROOT = pathlib.Path(__file__).resolve().parents[2]
BALANCE_H = ROOT / "Client/Public/BalanceTool.h"
BALANCE_CPP = ROOT / "Client/Private/BalanceTool.cpp"
PATTERN_TREE_CPP = ROOT / "Client/Private/ValtanPatternTree.cpp"
SERVER_TESTS = ROOT / "Server/Private/ServerGameplayContractTests.cpp"


def _pattern(master: dict, pattern_id: str) -> dict:
    return next(row for row in master["patterns"] if row["patternId"] == pattern_id)


def _stage(pattern: dict, stage_id: str) -> dict:
    return next(row for row in pattern["stages"] if row["stageId"] == stage_id)


def _flag_events(stage: dict, flag_id: str) -> list[dict]:
    return [
        event
        for event in stage["events"]
        if event.get("kind") == "SET_BOSS_FLAG" and event.get("flagId") == flag_id
    ]


class ValtanCounterAuthoringContractTests(unittest.TestCase):
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

    def apply(self, operation: dict, master: dict | None = None) -> dict:
        patch = {
            "schema": pipeline.DRAFT_PATCH_SCHEMA,
            "formatVersion": 1,
            "sourceRevision": self.source_revision,
            "operations": [operation],
        }
        candidate, _, _, count = pipeline.apply_draft_patch(
            copy.deepcopy(master if master is not None else self.master),
            copy.deepcopy(self.docs[pipeline.BOSS_PROFILES_REL]),
            copy.deepcopy(self.docs[pipeline.DAMAGE_REL]),
            patch,
            self.source_revision,
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        self.assertEqual(count, 1)
        return candidate

    @staticmethod
    def operation(
        *,
        pattern_id: str = "VALTAN_TRASH",
        stage_id: str = "STEP_07",
        enabled: bool = True,
        success_stage_id: str = "GROGGY",
        success_action_id: str = "valtan.sequence.center-trash-rush-if.groggy",
        timeout_stage_id: str | None = None,
        timeout_action_id: str | None = None,
    ) -> dict:
        if timeout_stage_id is None or timeout_action_id is None:
            if pattern_id == "VALTAN_TRASH_CATCH_IF":
                timeout_stage_id = "STEP_08"
                timeout_action_id = "valtan.sequence.rush-if.step-08"
            elif pattern_id == "VALTAN_DASH_CHARGE":
                timeout_stage_id = "CHARGE"
                timeout_action_id = "valtan.attack.dash-charge.active"
            else:
                timeout_stage_id = "STEP_08"
                timeout_action_id = "valtan.sequence.center-trash-rush-if.step-08"
        return {
            "op": "SET_STAGE_COUNTER_WINDOW",
            "patternId": pattern_id,
            "stageId": stage_id,
            "enabled": enabled,
            "successStageId": success_stage_id,
            "successActionId": success_action_id,
            "timeoutStageId": timeout_stage_id,
            "timeoutActionId": timeout_action_id,
        }

    @staticmethod
    def proxy_operation(
        *,
        pattern_id: str = "VALTAN_TRASH",
        stage_id: str = "STEP_07",
        forward_offset_m: float = 1.5,
        right_offset_m: float = -0.25,
        radius_m: float = 2.75,
    ) -> dict:
        return {
            "op": "SET_STAGE_COUNTER_PROXY",
            "patternId": pattern_id,
            "stageId": stage_id,
            "forwardOffsetM": forward_offset_m,
            "rightOffsetM": right_offset_m,
            "radiusM": radius_m,
        }

    def test_repository_counter_targets_are_closed_groggy_stages(self) -> None:
        gameplay = self.docs[pipeline.GAMEPLAY_AUTHORING_REL]
        expected = {
            "VALTAN_TRASH": ("STEP_07", "VALTAN_TRASH", "GROGGY"),
            "VALTAN_TRASH_CATCH_IF": (
                "STEP_07",
                "VALTAN_TRASH_CATCH_IF",
                "GROGGY",
            ),
            "VALTAN_COUNTER": (
                "STEP_02",
                "VALTAN_GROGGY_FOLLOWUP",
                "GROGGY",
            ),
        }
        for pattern_id, (source_id, target_pattern_id, target_id) in expected.items():
            pattern = _pattern({"patterns": gameplay["patterns"]}, pattern_id)
            source = _stage(pattern, source_id)
            target_pattern = _pattern(
                {"patterns": gameplay["patterns"]}, target_pattern_id
            )
            target = _stage(target_pattern, target_id)
            self.assertEqual(source["stageKind"], "WINDUP")
            self.assertEqual(target["stageKind"], "GROGGY")
            self.assertEqual(len(_flag_events(source, "boss.flag.counterable")), 2)
            self.assertEqual(len(_flag_events(target, "boss.flag.groggy")), 2)
            counter = [
                row for row in source["branches"] if row["outcome"] == "COUNTER_HIT"
            ]
            if pattern_id == target_pattern_id:
                self.assertEqual(
                    counter,
                    [{"outcome": "COUNTER_HIT", "nextActionId": target["actionId"]}],
                )
            else:
                self.assertEqual(
                    counter,
                    [{
                        "outcome": "COUNTER_HIT",
                        "nextActionId": None,
                        "nextPatternId": target_pattern_id,
                    }],
                )

    def test_typed_operation_disables_proxy_window_without_touching_groggy(self) -> None:
        cases = (
            (
                "VALTAN_TRASH",
                "valtan.sequence.center-trash-rush-if.groggy",
            ),
            (
                "VALTAN_TRASH_CATCH_IF",
                "valtan.sequence.rush-if.groggy",
            ),
        )
        for pattern_id, success_action_id in cases:
            with self.subTest(pattern_id=pattern_id):
                candidate = self.apply(
                    self.operation(
                        pattern_id=pattern_id,
                        enabled=False,
                        success_action_id=success_action_id,
                    )
                )
                pattern = _pattern(candidate, pattern_id)
                source = _stage(pattern, "STEP_07")
                target = _stage(pattern, "GROGGY")
                original_source = _stage(_pattern(self.master, pattern_id), "STEP_07")
                self.assertEqual(
                    original_source["counterProxy"],
                    source["counterProxy"],
                    "disabled authoring must retain the dormant BOSS_LOCAL geometry preset",
                )
                self.assertEqual(_flag_events(source, "boss.flag.counterable"), [])
                self.assertFalse(
                    any(row["outcome"] == "COUNTER_HIT" for row in source["branches"])
                )
                self.assertEqual(len(_flag_events(target, "boss.flag.groggy")), 2)
                product = pipeline.compile_pattern_product(candidate, pattern)
                product_source = _stage(product, "STEP_07")
                self.assertNotIn(
                    "counterProxy",
                    product_source,
                    "disabled Product must omit dormant proxy geometry",
                )

    def test_disable_save_reload_enable_save_preserves_exact_proxy(self) -> None:
        original = copy.deepcopy(
            _stage(_pattern(self.master, "VALTAN_TRASH"), "STEP_07")[
                "counterProxy"
            ]
        )
        with tempfile.TemporaryDirectory() as directory:
            authoring_root = pathlib.Path(directory) / "authoring"
            disabled = pipeline.save_authoring(
                ROOT,
                authoring_root,
                {
                    "schema": pipeline.DRAFT_PATCH_SCHEMA,
                    "formatVersion": 1,
                    "sourceRevision": self.source_revision,
                    "operations": [self.operation(enabled=False)],
                },
            )
            disabled_master, _, _ = pipeline.load_authoring_revision(
                ROOT,
                authoring_root,
                disabled["revisionId"],
                pipeline.source_manifest(ROOT),
                self.docs,
            )
            disabled_pattern = _pattern(disabled_master, "VALTAN_TRASH")
            self.assertEqual(original, _stage(disabled_pattern, "STEP_07")["counterProxy"])
            disabled_product = pipeline.compile_pattern_product(
                disabled_master, disabled_pattern
            )
            self.assertNotIn("counterProxy", _stage(disabled_product, "STEP_07"))

            enabled = pipeline.save_authoring(
                ROOT,
                authoring_root,
                {
                    "schema": pipeline.DRAFT_PATCH_SCHEMA,
                    "formatVersion": 1,
                    "sourceRevision": disabled["revisionId"],
                    "operations": [self.operation(enabled=True)],
                },
            )
            enabled_master, _, _ = pipeline.load_authoring_revision(
                ROOT,
                authoring_root,
                enabled["revisionId"],
                pipeline.source_manifest(ROOT),
                self.docs,
            )
            enabled_pattern = _pattern(enabled_master, "VALTAN_TRASH")
            self.assertEqual(original, _stage(enabled_pattern, "STEP_07")["counterProxy"])
            enabled_product = pipeline.compile_pattern_product(
                enabled_master, enabled_pattern
            )
            self.assertEqual(
                {
                    "kind": "BOSS_LOCAL_CIRCLE",
                    "forwardOffsetM": original["forwardOffsetM"],
                    "rightOffsetM": original["rightOffsetM"],
                    "radiusM": original["radiusM"],
                    "arcDegrees": 0.0,
                },
                _stage(enabled_product, "STEP_07")["counterProxy"],
            )

    def test_typed_operation_enables_selected_same_pattern_groggy(self) -> None:
        candidate = self.apply(self.operation())
        pattern = _pattern(candidate, "VALTAN_TRASH")
        source = _stage(pattern, "STEP_07")
        target = _stage(pattern, "GROGGY")
        self.assertEqual(len(_flag_events(source, "boss.flag.counterable")), 2)
        self.assertEqual(len(_flag_events(target, "boss.flag.groggy")), 2)
        self.assertEqual(
            [row for row in source["branches"] if row["outcome"] == "COUNTER_HIT"],
            [{"outcome": "COUNTER_HIT", "nextActionId": target["actionId"]}],
        )

    def test_typed_counter_proxy_operation_updates_only_exact_enabled_window(self) -> None:
        before = copy.deepcopy(self.master)
        candidate = self.apply(self.proxy_operation())
        proxy = _stage(_pattern(candidate, "VALTAN_TRASH"), "STEP_07")[
            "counterProxy"
        ]
        self.assertEqual(
            {
                "space": "BOSS_LOCAL",
                "forwardOffsetM": 1.5,
                "rightOffsetM": -0.25,
                "radiusM": 2.75,
            },
            proxy,
        )
        _stage(_pattern(candidate, "VALTAN_TRASH"), "STEP_07")[
            "counterProxy"
        ] = copy.deepcopy(
            _stage(_pattern(before, "VALTAN_TRASH"), "STEP_07")[
                "counterProxy"
            ]
        )
        self.assertEqual(before, candidate)

    def test_counter_proxy_rejects_wrong_stage_disabled_window_range_and_duplicate(self) -> None:
        disabled = self.apply(self.operation(enabled=False))
        invalid_cases = (
            (self.master, self.proxy_operation(stage_id="STEP_08")),
            (disabled, self.proxy_operation()),
            (self.master, self.proxy_operation(radius_m=0.0)),
            (self.master, self.proxy_operation(forward_offset_m=20.1)),
            (self.master, self.proxy_operation(right_offset_m=-20.1)),
        )
        for master, operation in invalid_cases:
            with self.subTest(operation=operation):
                with self.assertRaises(pipeline.DraftPatchError):
                    self.apply(operation, master)

        duplicate_patch = {
            "schema": pipeline.DRAFT_PATCH_SCHEMA,
            "formatVersion": 1,
            "sourceRevision": self.source_revision,
            "operations": [self.proxy_operation(), self.proxy_operation()],
        }
        with self.assertRaises(pipeline.DraftPatchError):
            pipeline.apply_draft_patch(
                copy.deepcopy(self.master),
                copy.deepcopy(self.docs[pipeline.BOSS_PROFILES_REL]),
                copy.deepcopy(self.docs[pipeline.DAMAGE_REL]),
                duplicate_patch,
                self.source_revision,
                self.docs[pipeline.WORLD_SET_REL],
                self.docs[pipeline.COMBAT_AUTHORING_REL],
            )

    def test_invalid_kind_cross_pattern_stale_duplicate_and_unpaired_reject(self) -> None:
        invalid_cases: list[tuple[dict, dict]] = []

        invalid_cases.append(
            (
                self.master,
                self.operation(stage_id="STEP_08"),
            )
        )
        invalid_cases.append(
            (
                self.master,
                self.operation(
                    success_stage_id="STEP_04",
                    success_action_id="valtan.sequence.counter.step-04",
                ),
            )
        )
        invalid_cases.append(
            (
                self.master,
                self.operation(success_action_id="valtan.missing.action"),
            )
        )

        duplicate = copy.deepcopy(self.master)
        duplicate_source = _stage(_pattern(duplicate, "VALTAN_TRASH"), "STEP_07")
        duplicate_source["branches"].append(copy.deepcopy(duplicate_source["branches"][0]))
        invalid_cases.append((duplicate, self.operation()))

        unpaired = copy.deepcopy(self.master)
        unpaired_target = _stage(_pattern(unpaired, "VALTAN_TRASH"), "GROGGY")
        unpaired_target["events"] = [
            event
            for event in unpaired_target["events"]
            if not (
                event.get("kind") == "SET_BOSS_FLAG"
                and event.get("flagId") == "boss.flag.groggy"
                and event.get("trigger") == "EXIT"
            )
        ]
        invalid_cases.append((unpaired, self.operation()))

        for master, operation in invalid_cases:
            with self.subTest(operation=operation):
                with self.assertRaises(pipeline.DraftPatchError):
                    self.apply(operation, master)

    def test_high_jump_airborne_hold_duration_remains_typed_stage_duration(self) -> None:
        candidate = self.apply(
            {
                "op": "SET_STAGE_DURATION",
                "patternId": "VALTAN_HIGH_JUMP",
                "stageId": "AIRBORNE",
                "durationMs": 6600,
            }
        )
        stage = _stage(_pattern(candidate, "VALTAN_HIGH_JUMP"), "AIRBORNE")
        self.assertEqual(stage["durationMs"], 6600)
        self.assertEqual(stage["animation"]["endPolicy"], "LOOP_TO_STAGE_END")
        self.assertTrue(stage["animation"]["occurrences"][0]["repeatUntilStageEnd"])

    def test_counter_save_failure_preserves_authoring_pointer(self) -> None:
        patch = {
            "schema": pipeline.DRAFT_PATCH_SCHEMA,
            "formatVersion": 1,
            "sourceRevision": self.source_revision,
            "operations": [self.operation(enabled=False)],
        }
        with tempfile.TemporaryDirectory() as directory:
            authoring_root = pathlib.Path(directory) / "authoring"
            with self.assertRaises(pipeline.InjectedFailure):
                pipeline.save_authoring(
                    ROOT,
                    authoring_root,
                    patch,
                    fail_at="after_stage",
                )
            self.assertFalse((authoring_root / "current-authoring.json").exists())
            self.assertEqual(list(authoring_root.glob(".stage.*")), [])

    def test_balance_ui_and_server_harness_use_typed_stable_id_contract(self) -> None:
        combined = BALANCE_H.read_text(encoding="utf-8") + BALANCE_CPP.read_text(encoding="utf-8")
        for marker in (
            "VALTAN_COUNTER_WINDOW_EDIT",
            "VALTAN_COUNTER_PROXY_EDIT",
            "Get_ValtanCounterWindowDraft",
            "Set_ValtanCounterWindowDraft",
            "Get_ValtanCounterProxyDraft",
            "Set_ValtanCounterProxyDraft",
            "SET_STAGE_COUNTER_WINDOW",
            "SET_STAGE_COUNTER_PROXY",
            "Counter window (Server authority)",
            "successStageId",
            "successActionId",
            "timeoutStageId",
            "timeoutActionId",
        ):
            self.assertIn(marker, combined)
        self.assertNotIn("stagedStage->CounterProxy.reset()", combined)
        self.assertIn("Counter Box area staged for", combined)
        pattern_tree = PATTERN_TREE_CPP.read_text(encoding="utf-8")
        self.assertIn("Validate_SplitCounterBranchContract", pattern_tree)
        self.assertIn("ProductCounterProxy", pattern_tree)
        server = SERVER_TESTS.read_text(encoding="utf-8")
        self.assertIn("Trash counter window fixed-tick admission", server)
        self.assertIn("outsideProxyRejected", server)
        self.assertIn("valtan.sequence.center-trash-rush-if.groggy", server)

    def test_balance_save_preserves_typed_cross_pattern_counter_followup(self) -> None:
        gameplay = self.docs[pipeline.GAMEPLAY_AUTHORING_REL]
        source = _stage(_pattern({"patterns": gameplay["patterns"]}, "VALTAN_COUNTER"), "STEP_02")
        self.assertEqual(
            [{
                "outcome": "COUNTER_HIT",
                "nextActionId": None,
                "nextPatternId": "VALTAN_GROGGY_FOLLOWUP",
            }],
            [row for row in source["branches"] if row["outcome"] == "COUNTER_HIT"],
        )
        pattern_ids = {row["patternId"] for row in gameplay["patterns"]}
        self.assertNotIn("VALTAN_COUNTER_GROGGY", pattern_ids)
        self.assertIn("VALTAN_COUNTER_GROGGY", gameplay["retiredPatternIds"])

        presentation = self.docs[pipeline.PRESENTATION_AUTHORING_REL]
        counter_p = _pattern({"patterns": presentation["patterns"]}, "VALTAN_COUNTER")
        animation = _stage(counter_p, "STEP_02")["animation"]
        self.assertEqual("EXACT", animation["endPolicy"])
        self.assertEqual(
            [
                ("valtan.sequence.counter.step-02.clip-01", 1000, False),
                ("valtan.sequence.counter.step-02.clip-02", 800, False),
            ],
            [(row["clipOccurrenceId"], row["playMs"],
              row["repeatUntilStageEnd"])
             for row in animation["occurrences"]],
        )
        self.assertTrue(all(
            row["clip"] == "mesh_att_battle_14_02"
            for row in animation["occurrences"]
        ))

        header = BALANCE_H.read_text(encoding="utf-8")
        balance = BALANCE_CPP.read_text(encoding="utf-8")
        for marker in (
            "successPatternId",
            "counterUsesPatternSuccess",
            "targetPattern->strEntryActionId",
            "Cross-Pattern Counter windows are preserved read-only",
        ):
            self.assertIn(marker, header + balance)


if __name__ == "__main__":
    unittest.main()
