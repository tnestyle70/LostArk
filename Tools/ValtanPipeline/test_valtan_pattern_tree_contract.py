from __future__ import annotations

import copy
import json
import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
MASTER_PATH = ROOT / "Data/Valtan/Valtan.pattern.json"
ENCOUNTER_PATH = ROOT / "Data/Encounters/Valtan/ValtanEncounter.json"
BINDINGS_PATH = ROOT / "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
ROTATIONS_PATH = ROOT / "Data/Encounters/Valtan/ValtanPatternRotations.json"
TREE_CPP = ROOT / "Client/Private/ValtanPatternTree.cpp"
TREE_HEADER = ROOT / "Client/Public/ValtanPatternTree.h"

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
    master: dict, encounter: dict, bindings: dict, rotations: dict
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
    rotation_by_id = {row["rotationId"]: row for row in rotations["rotations"]}
    for health_range in master["normalSelection"]["ranges"]:
        product = rotation_by_id.get(health_range["rotationId"])
        if product is None or product != {
            "rotationId": health_range["rotationId"],
            "selectionMode": "WEIGHTED_POOL",
            "fromHealthBar": health_range["fromHealthBar"],
            "toHealthBar": health_range["toHealthBar"],
            "patternIds": master["normalSelection"]["patternIds"],
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
        cls.encounter = load(ENCOUNTER_PATH)
        cls.bindings = load(BINDINGS_PATH)
        cls.rotations = load(ROTATIONS_PATH)
        cls.cpp = TREE_CPP.read_text(encoding="utf-8")
        cls.header = TREE_HEADER.read_text(encoding="utf-8")

    def test_current_managed_projection_is_exact(self) -> None:
        self.assertTrue(exact_managed_join(self.master, self.encounter))

    def test_current_master_only_contracts_are_admissible(self) -> None:
        self.assertTrue(master_only_contract_valid(self.master))

    def test_counter_reaction_and_weighted_selection_are_exact_product_joins(self) -> None:
        self.assertTrue(shared_reaction_and_selection_contract_valid(
            self.master, self.encounter, self.bindings, self.rotations
        ))

        unrelated = copy.deepcopy(self.master)
        unrelated["normalSelection"]["patternIds"][0] = "VALTAN_SWING"
        self.assertFalse(shared_reaction_and_selection_contract_valid(
            unrelated, self.encounter, self.bindings, self.rotations
        ))

        invented = copy.deepcopy(self.master)
        invented["counterReactionLayers"][0]["successActionId"] = "invented.action"
        self.assertFalse(shared_reaction_and_selection_contract_valid(
            invented, self.encounter, self.bindings, self.rotations
        ))

        for token in (
            "VALTAN_NORMAL_SELECTION_VIEW",
            "VALTAN_COUNTER_REACTION_LAYER_VIEW",
            "counter reaction master does not cover exact Product counter stages",
        ):
            self.assertIn(token, self.cpp + self.header)

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
