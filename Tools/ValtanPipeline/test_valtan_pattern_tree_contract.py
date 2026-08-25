from __future__ import annotations

import copy
import json
import re
import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
MASTER_PATH = ROOT / "Data/Valtan/Valtan.pattern.json"
GAMEPLAY_PATH = ROOT / "Data/Valtan/Valtan.gameplay.json"
PRESENTATION_PATH = ROOT / "Data/Valtan/Valtan.presentation.json"
ENCOUNTER_PATH = ROOT / "Data/Encounters/Valtan/ValtanEncounter.json"
BINDINGS_PATH = ROOT / "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
ROTATIONS_PATH = ROOT / "Data/Encounters/Valtan/ValtanPatternRotations.json"
TREE_CPP = ROOT / "Client/Private/ValtanPatternTree.cpp"
TREE_HEADER = ROOT / "Client/Public/ValtanPatternTree.h"
ENCOUNTER_REFERENCE_CPP = ROOT / "Client/Private/EncounterPatternReference.cpp"
VALTAN_LEVEL_CPP = ROOT / "Client/Private/Level_ValtanArena.cpp"
WORLD_SETS_PATH = ROOT / "Data/Valtan/Valtan.worldeventsets.json"
COMBAT_AUTHORING_PATH = ROOT / "Data/Valtan/Valtan.combatobjects.json"

sys.path.insert(0, str(ROOT / "Tools/ValtanPipeline"))
import valtan_tuning_pipeline as tuning_pipeline  # noqa: E402

PATTERN_FIELDS = (
    "patternId", "category", "minimumPhase", "maximumPhase",
    "targetPolicy", "aimPolicy", "displayName", "actionId",
    "sourceActionIds", "selectionMode", "minimumHealthBar",
    "maximumHealthBar", "triggerHealthBar", "triggerOrder",
    "armorRequirement", "phaseRequirement", "invulnerableWhileRunning",
    "selectionWeight", "maximumConsecutiveUses", "minimumRange",
    "maximumRange", "serverMotion",
)

STAGE_FIELDS = (
    "stageId", "actionId", "stageKind", "durationMs", "hitShape",
    "hitOuterRadius", "hitInnerRadius", "hitAngleDegrees", "hitLength",
    "hitHalfWidth", "hitCount", "hitIntervalMs", "hitDelayMs",
    "hitOffsetsMs", "serverDamageProfileId", "pushRangeM", "pushMs",
    "knockdown", "downMs", "motion", "actions", "branches",
)


def load(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def projected_value(row: dict, field: str):
    if field in {"serverMotion", "motion"}:
        return row.get(field)
    if field in {"hitOffsetsMs", "actions", "branches"}:
        return row.get(field, [])
    return row[field]


def exact_managed_join(master: dict, encounter: dict) -> bool:
    products = {row["patternId"]: row for row in encounter["patterns"]}
    for source_pattern in master["patterns"]:
        product_pattern = products.get(source_pattern["patternId"])
        if product_pattern is None:
            return False
        if any(
            projected_value(source_pattern, field)
            != projected_value(product_pattern, field)
            for field in PATTERN_FIELDS
        ):
            return False
        if len(source_pattern["stages"]) != len(product_pattern["stages"]):
            return False
        for source_stage, product_stage in zip(
            source_pattern["stages"], product_pattern["stages"], strict=True
        ):
            if any(
                projected_value(source_stage, field)
                != projected_value(product_stage, field)
                for field in STAGE_FIELDS
            ):
                return False
    return True


def split_join_is_strict(gameplay: dict, presentation: dict) -> bool:
    if any(
        gameplay.get(field) != presentation.get(field)
        for field in ("bossArchetypeId", "encounterId", "scope")
    ):
        return False
    gameplay_patterns = gameplay.get("patterns", [])
    presentation_patterns = presentation.get("patterns", [])
    gameplay_ids = [row.get("patternId") for row in gameplay_patterns]
    presentation_ids = [row.get("patternId") for row in presentation_patterns]
    if (
        gameplay_ids != presentation_ids
        or len(gameplay_ids) != len(set(gameplay_ids))
    ):
        return False
    for gameplay_pattern, presentation_pattern in zip(
        gameplay_patterns, presentation_patterns, strict=True
    ):
        gameplay_stages = gameplay_pattern.get("stages", [])
        presentation_stages = presentation_pattern.get("stages", [])
        gameplay_stage_ids = [row.get("stageId") for row in gameplay_stages]
        presentation_stage_ids = [
            row.get("stageId") for row in presentation_stages
        ]
        if (
            gameplay_stage_ids != presentation_stage_ids
            or len(gameplay_stage_ids) != len(set(gameplay_stage_ids))
        ):
            return False
        for gameplay_stage, presentation_stage in zip(
            gameplay_stages, presentation_stages, strict=True
        ):
            if gameplay_stage.get("actionId") != presentation_stage.get(
                "actionId"
            ):
                return False
            animation = presentation_stage.get("animation", {})
            occurrences = animation.get("occurrences", [])
            known_wall = sum(
                round(row["playMs"] / row["playRate"])
                for row in occurrences
                if row["playMs"] != 0
            )
            unknown = [row for row in occurrences if row["playMs"] == 0]
            duration = gameplay_stage.get("durationMs")
            end_policy = animation.get("endPolicy")
            if not isinstance(duration, int) or duration <= 0:
                return False
            if end_policy == "EXACT":
                if unknown or abs(known_wall - duration) > 2:
                    return False
            elif end_policy == "LOOP_TO_STAGE_END":
                if (
                    len(unknown) != 1
                    or not unknown[0].get("repeatUntilStageEnd")
                    or known_wall >= duration
                ):
                    return False
            elif end_policy == "HOLD_LAST_POSE":
                if len(unknown) > 1 or known_wall >= duration + 2:
                    return False
            else:
                return False
    return True


def split_policy_accepts(
    gameplay: dict,
    presentation: dict,
    encounter: dict,
    world_sets: dict,
    combat_authoring: dict,
    require_product_parity: bool,
) -> bool:
    try:
        joined = tuning_pipeline.join_v2_authoring(
            gameplay, presentation, world_sets, combat_authoring
        )
    except tuning_pipeline.PipelineError:
        return False
    products = {row["patternId"]: row for row in encounter["patterns"]}
    mechanics = {
        row["patternId"] for row in joined["decisionModel"]["mechanics"]
    }
    for pattern in joined["patterns"]:
        product = products.get(pattern["patternId"])
        if product is None:
            return False
        source_stage_topology = [
            (row["stageId"], row["actionId"]) for row in pattern["stages"]
        ]
        product_stage_topology = [
            (row["stageId"], row["actionId"]) for row in product["stages"]
        ]
        if (
            source_stage_topology != product_stage_topology
            or pattern["actionId"] != product["actionId"]
            or pattern["sourceActionIds"] != product["sourceActionIds"]
            or (pattern["patternId"] in mechanics)
            != (product["selectionMode"] == "HEALTH_BAR")
        ):
            return False
        if require_product_parity and (
            tuning_pipeline.compile_pattern_product(joined, pattern) != product
        ):
            return False
    return True


STABLE_TOKEN = re.compile(r"^[A-Za-z0-9_.-]+$")


def master_only_contract_valid(master: dict) -> bool:
    for pattern in master["patterns"]:
        stage_ids = {stage["stageId"] for stage in pattern["stages"]}
        reaction_keys: set[tuple[str, str]] = set()
        for reaction in pattern["reactions"]:
            key = (reaction["triggerKind"], reaction["stageId"])
            if (
                any(not STABLE_TOKEN.fullmatch(value) for value in key)
                or key in reaction_keys
                or reaction["stageId"] not in stage_ids
            ):
                return False
            reaction_keys.add(key)

        camera_cues = pattern["cameraCueIds"]
        if (
            len(camera_cues) != len(set(camera_cues))
            or any(not STABLE_TOKEN.fullmatch(cue) for cue in camera_cues)
        ):
            return False

        world_keys: set[tuple[str, str, str]] = set()
        for reference in pattern["worldEventTriggerRefs"]:
            key = (
                reference["patternId"], reference["stageId"],
                reference["triggerKind"],
            )
            if (
                any(not STABLE_TOKEN.fullmatch(value) for value in key)
                or key in world_keys
                or reference["patternId"] != pattern["patternId"]
                or reference["stageId"] not in stage_ids
            ):
                return False
            world_keys.add(key)

        for stage in pattern["stages"]:
            animation = stage["animation"]
            if animation["repeatCount"] <= 1:
                continue
            occurrences = animation["occurrences"]
            if (
                len(occurrences) != animation["repeatCount"]
                or len({row["clip"] for row in occurrences}) != 1
            ):
                return False
    return True


def shared_reaction_and_selection_contract_valid(
    master: dict, gameplay: dict, encounter: dict, bindings: dict,
    rotations: dict
) -> bool:
    managed_ids = {row["patternId"] for row in master["patterns"]}
    managed_normal_ids = {
        row["patternId"] for row in master["patterns"]
        if row["selectionMode"] == "NORMAL"
    }
    if (
        master["normalSelection"]["selectionMode"] != "WEIGHTED_POOL"
        or set(master["normalSelection"]["patternIds"]) != managed_normal_ids
    ):
        return False
    if rotations.get("formatVersion") != 3:
        return False
    managed_rotation_ids = [
        row["compatibilityRotationId"]
        for row in gameplay["decisionModel"]["selectionWindows"]
    ]
    rotation_rows = rotations["rotations"]
    if (
        [row.get("rotationId") for row in rotation_rows[:len(managed_rotation_ids)]]
        != managed_rotation_ids
        or len(rotation_rows[len(managed_rotation_ids):]) != 6
        or any(
            row.get("selectionMode") != "ORDERED_INTRO_THEN_WEIGHTED"
            for row in rotation_rows[len(managed_rotation_ids):]
        )
    ):
        return False
    rotation_by_id = {row["rotationId"]: row for row in rotations["rotations"]}
    sets = {
        row["selectionSetId"]: row
        for row in gameplay["decisionModel"]["selectionSets"]
    }
    for window in gameplay["decisionModel"]["selectionWindows"]:
        selection_set = sets.get(window["selectionSetId"])
        product = rotation_by_id.get(window["compatibilityRotationId"])
        if selection_set is None or product is None or product != {
            "rotationId": window["compatibilityRotationId"],
            "selectionMode": selection_set["mode"],
            "fromHealthBar": window["maximumHealthBarInclusive"],
            "toHealthBar": window["minimumHealthBarExclusive"],
            "windowId": window["windowId"],
            "gameplayPhase": window["gameplayPhase"],
            "selectionSetId": window["selectionSetId"],
            "candidates": selection_set["candidates"],
        }:
            return False

    patterns = {row["patternId"]: row for row in encounter["patterns"]}
    binding_actions = {
        row["actionId"] for row in bindings["bindings"] if row["clips"]
    }
    product_counter_stages: set[tuple[str, str]] = set()
    for pattern in encounter["patterns"]:
        for stage in pattern["stages"]:
            if any(
                action.get("trigger") == "ENTER"
                and action.get("kind") == "SET_BOSS_FLAG"
                and action.get("targetId") == "boss.flag.counterable"
                and action.get("value") == 1
                for action in stage.get("actions", [])
            ):
                product_counter_stages.add((pattern["patternId"], stage["stageId"]))

    master_counter_stages: set[tuple[str, str]] = set()
    for layer in master["counterReactionLayers"]:
        if (
            layer["admissionScope"] != "REFERENCE_ONLY_LEGACY"
            or layer["ownerPatternId"] in managed_ids
            or layer["reactionLayerId"] == ""
        ):
            return False
        owner = patterns.get(layer["ownerPatternId"])
        if owner is None:
            return False
        stages_by_id = {stage["stageId"]: stage for stage in owner["stages"]}
        stages_by_action = {stage["actionId"]: stage for stage in owner["stages"]}
        window = stages_by_id.get(layer["ownerStageId"])
        if window is None or window["actionId"] != layer["windowActionId"]:
            return False
        branches = {
            row["outcome"]: row.get("nextActionId")
            for row in window.get("branches", [])
        }
        actions = (
            layer["windowActionId"], layer["successActionId"],
            layer["failureActionId"],
        )
        if (
            branches.get("COUNTER_HIT") != layer["successActionId"]
            or branches.get("TIMEOUT") != layer["failureActionId"]
            or any(action not in stages_by_action for action in actions)
            or any(action not in binding_actions for action in actions)
        ):
            return False
        master_counter_stages.add(
            (layer["ownerPatternId"], layer["ownerStageId"])
        )
    return master_counter_stages == product_counter_stages


class ValtanPatternTreeContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.master = load(MASTER_PATH)
        cls.gameplay = load(GAMEPLAY_PATH)
        cls.presentation = load(PRESENTATION_PATH)
        cls.world_sets = load(WORLD_SETS_PATH)
        cls.combat_authoring = load(COMBAT_AUTHORING_PATH)
        cls.encounter = load(ENCOUNTER_PATH)
        cls.bindings = load(BINDINGS_PATH)
        cls.rotations = load(ROTATIONS_PATH)
        cls.cpp = TREE_CPP.read_text(encoding="utf-8")
        cls.header = TREE_HEADER.read_text(encoding="utf-8")
        cls.encounter_reference_cpp = ENCOUNTER_REFERENCE_CPP.read_text(
            encoding="utf-8"
        )
        cls.valtan_level_cpp = VALTAN_LEVEL_CPP.read_text(encoding="utf-8")

    def test_v1_monolith_is_migration_fixture_not_runtime_admission(self) -> None:
        self.assertEqual(
            self.gameplay["previewPaths"]["legacyCompatibility"],
            "Data/Valtan/Valtan.legacy-compatibility.json",
        )
        self.assertNotIn('L"Valtan.pattern.json"', self.cpp)

    def test_cpp_uses_physical_split_authoring_and_keeps_v1_as_fixture(self) -> None:
        self.assertTrue(split_join_is_strict(self.gameplay, self.presentation))
        self.assertIn("Valtan.gameplay.json", self.cpp)
        self.assertIn("Valtan.presentation.json", self.cpp)
        self.assertIn("Load_FromAuthoringPaths", self.cpp + self.header)
        self.assertIn("Parse_SplitMasterDocument", self.cpp)
        self.assertNotIn('L"Valtan.pattern.json"', self.cpp)

    def test_level_audition_reads_rotation_v3_candidates(self) -> None:
        self.assertIn('rotation.Find("candidates")', self.valtan_level_cpp)
        self.assertIn('candidate.Find("patternId")', self.valtan_level_cpp)
        self.assertIn('candidate.Find("enabled")', self.valtan_level_cpp)

    def test_split_identity_action_and_wall_drift_fail_closed(self) -> None:
        mutations: list[tuple[str, dict, dict]] = []

        missing = copy.deepcopy(self.presentation)
        missing["patterns"].pop()
        mutations.append(("missing pattern", self.gameplay, missing))

        duplicate = copy.deepcopy(self.presentation)
        duplicate["patterns"][1]["patternId"] = duplicate["patterns"][0][
            "patternId"
        ]
        mutations.append(("duplicate pattern", self.gameplay, duplicate))

        extra = copy.deepcopy(self.presentation)
        extra_stage = copy.deepcopy(extra["patterns"][0]["stages"][-1])
        extra_stage["stageId"] = "EXTRA_STAGE"
        extra_stage["actionId"] = "valtan.test.extra-stage"
        extra["patterns"][0]["stages"].append(extra_stage)
        mutations.append(("extra stage", self.gameplay, extra))

        action = copy.deepcopy(self.presentation)
        action["patterns"][0]["stages"][0]["actionId"] = "drift.action"
        mutations.append(("action mismatch", self.gameplay, action))

        duration = copy.deepcopy(self.gameplay)
        arena = next(
            row for row in duration["patterns"]
            if row["patternId"] == "VALTAN_ARENA_BREAK_109"
        )
        takeoff = next(
            row for row in arena["stages"] if row["stageId"] == "TAKEOFF"
        )
        takeoff["durationMs"] += 10
        mutations.append(("animation wall", duration, self.presentation))

        for name, gameplay, presentation in mutations:
            with self.subTest(name=name):
                self.assertFalse(split_join_is_strict(gameplay, presentation))

        for token in (
            "split authoring patternId closure mismatch",
            "split authoring stageId/actionId mismatch",
            "split presentation/Server stage wall join failed",
        ):
            self.assertIn(token, self.cpp)

    def test_saved_gameplay_overlay_policy_differs_from_product_parity(self) -> None:
        tuned = copy.deepcopy(self.gameplay)
        whirlwind = next(
            row for row in tuned["patterns"]
            if row["patternId"] == "VALTAN_WHIRLWIND"
        )
        whirlwind["eligibility"]["minimumRangeM"] = 0.5

        self.assertTrue(split_policy_accepts(
            tuned,
            self.presentation,
            self.encounter,
            self.world_sets,
            self.combat_authoring,
            require_product_parity=False,
        ))
        self.assertFalse(split_policy_accepts(
            tuned,
            self.presentation,
            self.encounter,
            self.world_sets,
            self.combat_authoring,
            require_product_parity=True,
        ))

        orphaned_cue_owner = copy.deepcopy(self.presentation)
        pattern_stage_independent = next(
            row for row in orphaned_cue_owner["independentEffects"]
            if row["ownership"] == "SERVER_PATTERN_STAGE"
        )
        pattern_stage_independent["cueId"] = "cue.valtan.missing"
        self.assertFalse(split_policy_accepts(
            self.gameplay,
            orphaned_cue_owner,
            self.encounter,
            self.world_sets,
            self.combat_authoring,
            require_product_parity=False,
        ))

        for token in (
            "REQUIRE_ACTIVE_PRODUCT_PARITY",
            "RESTORE_AUTHORING_SNAPSHOT",
            "Has_StableRestoreTopology",
            "Overlay_MasterGameplay",
        ):
            self.assertIn(token, self.cpp + self.header)

    def test_empty_then_one_operation_save_resumes_exact_split_revision(self) -> None:
        source_manifest = tuning_pipeline.source_manifest(ROOT)
        repository_join = tuning_pipeline.join_v2_authoring(
            self.gameplay,
            self.presentation,
            self.world_sets,
            self.combat_authoring,
        )
        tuned_pattern = next(
            row
            for row in repository_join["patterns"]
            if row["patternId"] == "VALTAN_WHIRLWIND"
        )
        tuned_stage = next(
            row for row in tuned_pattern["stages"] if row["stageId"] == "WINDUP"
        )
        tuned_duration = tuned_stage["durationMs"] + 1
        legacy_before = MASTER_PATH.read_bytes()

        with tempfile.TemporaryDirectory() as temporary:
            authoring_root = Path(temporary) / "authoring"
            empty_patch = {
                "schema": tuning_pipeline.DRAFT_PATCH_SCHEMA,
                "formatVersion": 1,
                "sourceRevision": source_manifest["sourceManifestId"],
                "operations": [],
            }
            empty_pointer = tuning_pipeline.save_authoring(
                ROOT, authoring_root, empty_patch
            )
            empty_master, _, _ = tuning_pipeline.load_authoring_revision(
                ROOT,
                authoring_root,
                empty_pointer["revisionId"],
                source_manifest,
                tuning_pipeline.load_pipeline_documents(ROOT),
            )
            self.assertEqual(repository_join, empty_master)

            one_operation_patch = {
                "schema": tuning_pipeline.DRAFT_PATCH_SCHEMA,
                "formatVersion": 1,
                "sourceRevision": empty_pointer["revisionId"],
                "operations": [
                    {
                        "op": "SET_STAGE_DURATION",
                        "patternId": tuned_pattern["patternId"],
                        "stageId": tuned_stage["stageId"],
                        "durationMs": tuned_duration,
                    }
                ],
            }
            tuned_pointer = tuning_pipeline.save_authoring(
                ROOT, authoring_root, one_operation_patch
            )
            tuned_revision_root = (
                authoring_root / "revisions" / tuned_pointer["revisionId"]
            )
            tuned_gameplay = load(
                tuned_revision_root / tuning_pipeline.GAMEPLAY_AUTHORING_REL
            )
            tuned_presentation = load(
                tuned_revision_root / tuning_pipeline.PRESENTATION_AUTHORING_REL
            )
            tuned_master, _, _ = tuning_pipeline.load_authoring_revision(
                ROOT,
                authoring_root,
                tuned_pointer["revisionId"],
                source_manifest,
                tuning_pipeline.load_pipeline_documents(ROOT),
            )
            resumed_duration = next(
                source_stage["durationMs"]
                for source_pattern in tuned_master["patterns"]
                if source_pattern["patternId"] == tuned_pattern["patternId"]
                for source_stage in source_pattern["stages"]
                if source_stage["stageId"] == tuned_stage["stageId"]
            )
            self.assertEqual(tuned_duration, resumed_duration)
            self.assertEqual(
                tuned_master,
                tuning_pipeline.join_v2_authoring(
                    tuned_gameplay,
                    tuned_presentation,
                    self.world_sets,
                    self.combat_authoring,
                ),
            )
            self.assertTrue(split_policy_accepts(
                tuned_gameplay,
                tuned_presentation,
                self.encounter,
                self.world_sets,
                self.combat_authoring,
                require_product_parity=False,
            ))
            self.assertFalse(split_policy_accepts(
                tuned_gameplay,
                tuned_presentation,
                self.encounter,
                self.world_sets,
                self.combat_authoring,
                require_product_parity=True,
            ))
            _, source_payload, effective_revision = (
                tuning_pipeline.source_manifest_with_authoring(
                    ROOT, authoring_root
                )
            )
            self.assertEqual(tuned_pointer["revisionId"], effective_revision)
            self.assertEqual(
                tuned_pointer["revisionId"],
                source_payload["authoringRevision"],
            )

        self.assertEqual(legacy_before, MASTER_PATH.read_bytes())

    def test_current_master_only_contracts_are_admissible(self) -> None:
        self.assertTrue(master_only_contract_valid(self.master))

    def test_counter_reaction_and_weighted_selection_are_exact_product_joins(self) -> None:
        self.assertTrue(shared_reaction_and_selection_contract_valid(
            self.master, self.gameplay, self.encounter, self.bindings,
            self.rotations
        ))

        unrelated = copy.deepcopy(self.master)
        unrelated["normalSelection"]["patternIds"][0] = "VALTAN_SWING"
        self.assertFalse(shared_reaction_and_selection_contract_valid(
            unrelated, self.gameplay, self.encounter, self.bindings,
            self.rotations
        ))

        invented = copy.deepcopy(self.master)
        invented["counterReactionLayers"][0]["successActionId"] = "invented.action"
        self.assertFalse(shared_reaction_and_selection_contract_valid(
            invented, self.gameplay, self.encounter, self.bindings,
            self.rotations
        ))

        for token in (
            "VALTAN_NORMAL_SELECTION_VIEW",
            "VALTAN_SELECTION_SET_VIEW",
            "VALTAN_SELECTION_WINDOW_VIEW",
            "VALTAN_MECHANIC_VIEW",
            "VALTAN_LEGACY_ROTATION_VIEW",
            "VALTAN_ARENA_BREAK_109 health bar must match the final phase-1 window boundary",
            "VALTAN_COUNTER_REACTION_LAYER_VIEW",
            "counter reaction master does not cover exact Product counter stages",
        ):
            self.assertIn(token, self.cpp + self.header)

    def test_split_keeps_per_set_weight_enabled_and_legacy_rows_lossless(self) -> None:
        tuned = copy.deepcopy(self.gameplay)
        first = tuned["decisionModel"]["selectionSets"][0]["candidates"][1]
        second = tuned["decisionModel"]["selectionSets"][1]["candidates"][1]
        first["weight"] = second["weight"] + 1
        second["enabled"] = False
        joined = tuning_pipeline.join_v2_authoring(
            tuned, self.presentation, self.world_sets, self.combat_authoring
        )
        self.assertEqual(
            second["weight"] + 1,
            joined["decisionModel"]["selectionSets"][0]["candidates"][1]["weight"],
        )
        self.assertFalse(
            joined["decisionModel"]["selectionSets"][1]["candidates"][1]["enabled"]
        )
        self.assertNotIn(
            "split gameplay pattern weight differs between selection sets",
            self.cpp,
        )
        legacy = self.rotations["rotations"][2:]
        self.assertEqual(6, len(legacy))
        self.assertEqual(
            ["VALTAN_EARTHQUAKE_SMASH", "VALTAN_EARTHQUAKE_SMASH", "VALTAN_DASH_CHARGE"],
            legacy[0]["patternIds"],
        )
        reordered = copy.deepcopy(self.rotations)
        reordered["rotations"][0], reordered["rotations"][2] = (
            reordered["rotations"][2], reordered["rotations"][0]
        )
        self.assertFalse(shared_reaction_and_selection_contract_valid(
            self.master, self.gameplay, self.encounter, self.bindings, reordered
        ))

    def test_v4_action_validator_admits_authored_volley_and_phase_transition(self) -> None:
        actions = [
            action
            for pattern in self.encounter["patterns"]
            for stage in pattern["stages"]
            for action in stage.get("actions", [])
        ]
        volleys = [
            action for action in actions
            if action.get("kind") == "SPAWN_COMBAT_OBJECT_VOLLEY"
        ]
        phase_changes = [
            action for action in actions
            if action.get("kind") == "SET_GAMEPLAY_PHASE"
        ]
        self.assertTrue(volleys)
        self.assertEqual(1, len(phase_changes))
        for volley in volleys:
            self.assertEqual({
                "trigger", "kind", "targetId", "targetingPolicy",
                "countPerResolvedTarget", "layout", "radiusM",
                "startAngleDegrees", "angleStepDegrees", "allowOverlap",
                "maximumTotalObjects",
            }, set(volley))
            self.assertEqual("ENTER", volley["trigger"])
            self.assertEqual("PER_ALIVE_PLAYER", volley["targetingPolicy"])
            self.assertGreaterEqual(
                volley["maximumTotalObjects"], volley["countPerResolvedTarget"]
            )
        self.assertEqual({
            "trigger": "ENTER",
            "kind": "SET_GAMEPLAY_PHASE",
            "targetId": "boss.phase.gameplay",
            "value": 2,
            "durationMs": 0,
        }, phase_changes[0])
        for marker in (
            'actionKind->Get_String() == "SPAWN_COMBAT_OBJECT_VOLLEY"',
            'kind == "SET_GAMEPLAY_PHASE"',
            'targetingPolicy != "PER_ALIVE_PLAYER"',
            'targetId == "boss.phase.gameplay" && 2u == value',
        ):
            self.assertIn(marker, self.encounter_reference_cpp)

    def test_gameplay_and_branch_mutations_fail_closed(self) -> None:
        mutations = (
            ("VALTAN_WHIRLWIND", "SPIN", "hitOuterRadius", 999.0),
            ("VALTAN_WHIRLWIND", "SPIN", "serverDamageProfileId", "drift"),
            ("VALTAN_DASH_CHARGE", "CHARGE", "motion", {"kind": "FORWARD", "distance": 19.0}),
            ("VALTAN_DASH_CHARGE", "GROGGY", "actions", []),
            ("VALTAN_DASH_CHARGE", "CHARGE", "branches", []),
        )
        for pattern_id, stage_id, field, value in mutations:
            with self.subTest(field=field):
                drifted = copy.deepcopy(self.encounter)
                pattern = next(
                    row for row in drifted["patterns"]
                    if row["patternId"] == pattern_id
                )
                stage = next(
                    row for row in pattern["stages"]
                    if row["stageId"] == stage_id
                )
                stage[field] = value
                self.assertFalse(exact_managed_join(self.master, drifted))

    def test_pattern_selection_and_server_motion_mutations_fail_closed(self) -> None:
        for field, value in (
            ("selectionWeight", 999),
            ("sourceActionIds", [1]),
            ("serverMotion", None),
        ):
            with self.subTest(field=field):
                drifted = copy.deepcopy(self.encounter)
                pattern = next(
                    row for row in drifted["patterns"]
                    if row["patternId"] == "VALTAN_HIGH_JUMP"
                )
                pattern[field] = value
                self.assertFalse(exact_managed_join(self.master, drifted))

    def test_cpp_retains_every_projected_field_and_ordered_branches(self) -> None:
        self.assertIn("Equal_MasterPatternGameplay", self.cpp)
        self.assertIn("Equal_MasterStageGameplay", self.cpp)
        for token in (
            "SourceActionIds", "ServerMotion", "HitOffsetsMs", "Actions",
            "Branches", "strNextActionId", "strAnimationEndPolicy",
            "PresentationSources",
        ):
            self.assertIn(token, self.cpp + self.header)

    def test_master_only_contract_mutations_fail_closed(self) -> None:
        mutations = []

        reaction = copy.deepcopy(self.master)
        pattern = next(
            row for row in reaction["patterns"]
            if row["patternId"] == "VALTAN_DASH_CHARGE"
        )
        pattern["reactions"][0]["stageId"] = "UNKNOWN_STAGE"
        mutations.append(("reaction stage", reaction))

        camera = copy.deepcopy(self.master)
        pattern = next(
            row for row in camera["patterns"]
            if row["patternId"] == "VALTAN_ARENA_BREAK_109"
        )
        pattern["cameraCueIds"].append(pattern["cameraCueIds"][0])
        mutations.append(("camera duplicate", camera))

        world = copy.deepcopy(self.master)
        pattern = next(
            row for row in world["patterns"]
            if row["patternId"] == "VALTAN_ARENA_BREAK_109"
        )
        pattern["worldEventTriggerRefs"][0]["patternId"] = "OTHER_PATTERN"
        mutations.append(("world reference owner", world))

        repeat_clip = copy.deepcopy(self.master)
        pattern = next(
            row for row in repeat_clip["patterns"]
            if row["patternId"] == "VALTAN_DASH_CHARGE"
        )
        stage = next(
            row for row in pattern["stages"] if row["stageId"] == "WINDUP"
        )
        stage["animation"]["occurrences"][1]["clip"] = "other_clip"
        mutations.append(("repeat clip", repeat_clip))

        repeat_count = copy.deepcopy(self.master)
        pattern = next(
            row for row in repeat_count["patterns"]
            if row["patternId"] == "VALTAN_DASH_CHARGE"
        )
        stage = next(
            row for row in pattern["stages"] if row["stageId"] == "WINDUP"
        )
        stage["animation"]["repeatCount"] = 2
        mutations.append(("repeat occurrence count", repeat_count))

        for name, drifted in mutations:
            with self.subTest(name=name):
                self.assertFalse(master_only_contract_valid(drifted))

        for token in (
            "VALTAN_PATTERN_REACTION_VIEW", "CameraCueIds",
            "VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW",
            "repeatCount occurrences must use one clip",
        ):
            self.assertIn(token, self.cpp + self.header)

    def test_preview_path_is_graph_driven_not_stage_id_driven(self) -> None:
        begin = self.cpp.index("CValtanPatternTree::Build_PreviewStagePath")
        end = self.cpp.index("CValtanPatternTree::Load", begin)
        body = self.cpp[begin:end]
        for forbidden_stage_literal in (
            '"WINDUP"', '"CHARGE"', '"GROGGY"', '"RECOVERY"',
            '"PART_BREAK"',
        ):
            self.assertNotIn(forbidden_stage_literal, body)
        for outcome in ("TIMEOUT", "WALL_CONTACT", "PART_DESTROYED"):
            self.assertIn(f'"{outcome}"', body)
        for guard in ("contains a cycle", "unknown action", "ambiguous"):
            self.assertIn(guard, body)


if __name__ == "__main__":
    unittest.main()
