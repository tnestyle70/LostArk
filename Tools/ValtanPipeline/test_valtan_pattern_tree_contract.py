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
BALANCE_TOOL_CPP = ROOT / "Client/Private/BalanceTool.cpp"
EFFECT_CUE_CPP = ROOT / "Client/Private/ValtanPatternEffectCueDocument.cpp"
EFFECT_SERVICE_CPP = ROOT / "Client/Private/Effect_PresentationService.cpp"
EFFECT_TOOL_CPP = ROOT / "Client/Private/Effect_Tool.cpp"
ENCOUNTER_REFERENCE_CPP = ROOT / "Client/Private/EncounterPatternReference.cpp"
VALTAN_LEVEL_CPP = ROOT / "Client/Private/Level_ValtanArena.cpp"
PATTERN_MASTER_PROJECTOR = (
    ROOT / "Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1"
)
WORLD_SETS_PATH = ROOT / "Data/Valtan/Valtan.worldeventsets.json"
COMBAT_AUTHORING_PATH = ROOT / "Data/Valtan/Valtan.combatobjects.json"
PROMOTION_MANIFEST_PATH = (
    ROOT / "Data/Valtan/Valtan.animation-chain-promotions.json"
)
PROMOTION_RECEIPT_PATH = (
    ROOT / "Data/Valtan/Valtan.animation-chain-promotion.receipt.json"
)

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
    document = json.loads(path.read_text(encoding="utf-8"))
    if path.name == "Valtan.gameplay.json" and "flowId" in document["decisionModel"]["scriptedSequence"]:
        return tuning_pipeline.resolve_gameplay_flow_reference(
            document, load(path.parent.parent / "Encounters/Valtan/ValtanBossAuditionFlows.json"),
        )
    return document


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
            animation_mode = animation.get("mode", "CLIP_SEQUENCE")
            if animation_mode == "NONE":
                if set(animation) != {"mode"}:
                    return False
                continue
            if animation_mode != "CLIP_SEQUENCE":
                return False
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
    if not require_product_parity:
        return True
    products = {row["patternId"]: row for row in encounter["patterns"]}
    mechanics = {
        row["patternId"] for row in joined["decisionModel"]["mechanics"]
    }
    manual_auditions = {
        row["patternId"]
        for row in joined["decisionModel"]["manualAuditions"]
    }
    for pattern in joined["patterns"]:
        product = products.get(pattern["patternId"])
        if product is None:
            return False
        if product["selectionMode"] != (
            "HEALTH_BAR"
            if pattern["patternId"] in mechanics
            else "AUDITION_ONLY"
            if pattern["patternId"] in manual_auditions
            else "NORMAL"
        ):
            return False
        if require_product_parity:
            source_stage_topology = [
                (row["stageId"], row["actionId"])
                for row in pattern["stages"]
            ]
            product_stage_topology = [
                (row["stageId"], row["actionId"])
                for row in product["stages"]
            ]
            if (
                source_stage_topology != product_stage_topology
                or pattern["actionId"] != product["actionId"]
                or pattern["sourceActionIds"] != product["sourceActionIds"]
                or tuning_pipeline.compile_pattern_product(joined, pattern)
                != product
            ):
                return False
    return True


STABLE_TOKEN = re.compile(r"^[A-Za-z0-9_.-]+$")

SAVED_DEFAULT_FLOW = load(ROOT / tuning_pipeline.SAVED_FLOW_REL)["flows"][0]
EXPECTED_SCRIPTED_SEQUENCE = {
    "sequenceId": "sequence.valtan.server-authored.v1",
    "mode": "ORDERED_ONCE_THEN_IDLE",
    "interStepPursuitMs": SAVED_DEFAULT_FLOW["interStepPursuitMs"],
    "patternIds": [slot["patternId"] for slot in SAVED_DEFAULT_FLOW["slots"]],
}

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
    scripted_sequence = gameplay["decisionModel"].get("scriptedSequence")
    gameplay_ids = {row["patternId"] for row in gameplay["patterns"]}
    product_ids = {row["patternId"] for row in encounter["patterns"]}
    if (
        set(rotations) != {
            "schema", "formatVersion", "encounterId", "bossArchetypeId",
            "scriptedSequence", "rotations",
        }
        or rotations.get("formatVersion") != 4
        or scripted_sequence != EXPECTED_SCRIPTED_SEQUENCE
        or rotations.get("scriptedSequence") != scripted_sequence
        or scripted_sequence["patternIds"]
        != EXPECTED_SCRIPTED_SEQUENCE["patternIds"]
        or any(
            pattern_id not in gameplay_ids or pattern_id not in product_ids
            for pattern_id in scripted_sequence["patternIds"]
        )
    ):
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
        if pattern["patternId"] in gameplay_ids:
            continue
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
        cls.promotion_manifest = load(PROMOTION_MANIFEST_PATH)
        cls.promotion_receipt = load(PROMOTION_RECEIPT_PATH)
        cls.encounter = load(ENCOUNTER_PATH)
        cls.bindings = load(BINDINGS_PATH)
        cls.rotations = load(ROTATIONS_PATH)
        cls.cpp = TREE_CPP.read_text(encoding="utf-8")
        cls.header = TREE_HEADER.read_text(encoding="utf-8")
        cls.balance_tool_cpp = BALANCE_TOOL_CPP.read_text(encoding="utf-8")
        cls.effect_cue_cpp = EFFECT_CUE_CPP.read_text(encoding="utf-8")
        cls.effect_service_cpp = EFFECT_SERVICE_CPP.read_text(encoding="utf-8")
        cls.effect_tool_cpp = EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        cls.encounter_reference_cpp = ENCOUNTER_REFERENCE_CPP.read_text(
            encoding="utf-8"
        )
        cls.valtan_level_cpp = VALTAN_LEVEL_CPP.read_text(encoding="utf-8")
        cls.pattern_master_projector = PATTERN_MASTER_PROJECTOR.read_text(
            encoding="utf-8"
        )

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

    def test_next_inventory_follows_all_split_owned_product_identities(self) -> None:
        gameplay_ids = {row["patternId"] for row in self.gameplay["patterns"]}
        presentation_ids = {row["patternId"] for row in self.presentation["patterns"]}
        product_by_id = {row["patternId"]: row for row in self.encounter["patterns"]}
        self.assertEqual(gameplay_ids, presentation_ids)
        self.assertLessEqual(gameplay_ids, product_by_id.keys())
        self.assertIn("VALTAN_FIST_IN_OUT", gameplay_ids)
        for pattern in self.gameplay["patterns"]:
            with self.subTest(pattern=pattern["patternId"]):
                self.assertTrue(pattern["stages"])
                self.assertEqual(pattern["entryActionId"], pattern["stages"][0]["actionId"])
                product = product_by_id[pattern["patternId"]]
                self.assertEqual(
                    [(row["stageId"], row["actionId"]) for row in pattern["stages"]],
                    [(row["stageId"], row["actionId"]) for row in product["stages"]],
                )
        begin = self.cpp.index("CValtanPatternTree::Build_PlayablePatternInventory(")
        end = self.cpp.index("CValtanPatternTree::Build_NextPatternInventory(", begin)
        builder = self.cpp[begin:end]
        for marker in (
            "&View.Gimmicks, &View.Rotation", "bAuthoringMasterManaged",
            "IdentityCounts", "strEntryActionId", "StageIds", "ActionIds",
            "ManualByPattern", "OutInventory = std::move(Staged)",
        ):
            self.assertIn(marker, builder)
        self.assertNotIn("TOTAL_PATTERN_COUNT", builder)
        self.assertNotIn("ANIMATOR_PATTERN_COUNT", builder)
        self.assertNotIn("VALTAN_FIST_IN_OUT", builder)
        self.assertLess(builder.index("StagedPatterns.empty()"), builder.index("OutInventory ="))
        next_end = self.cpp.index("CValtanPatternTree::Build_PatternIdentitySummary(", end)
        wrapper = self.cpp[end:next_end]
        self.assertIn("Build_PlayablePatternInventory(View, Inventory, strOutError)", wrapper)
        self.assertIn("iSourceSequenceIndex", wrapper)
        self.assertLess(wrapper.index("Build_PlayablePatternInventory("), wrapper.index("OutPatternIds ="))

    def test_trash_terminal_actions_keep_typed_source_product_join(self) -> None:
        source = next(row for row in self.gameplay["patterns"] if row["patternId"] == "VALTAN_TRASH")
        product = next(row for row in self.encounter["patterns"] if row["patternId"] == "VALTAN_TRASH")
        stages = {row["stageId"]: row for row in product["stages"]}
        seen = set()
        for stage in source["stages"]:
            for event in stage["events"]:
                if event["kind"] not in {"DAMAGE_GRABBED_PLAYERS", "EXECUTE_GRABBED_PLAYERS"}:
                    continue
                seen.add(event["kind"])
                expected = {
                    "trigger": "ENTER", "kind": event["kind"], "value": 0, "durationMs": 0,
                    "targetId": event.get("damageProfileId", "boss.attachment.left-hand"),
                }
                self.assertIn(expected, stages[stage["stageId"]]["actions"])
        self.assertEqual({"DAMAGE_GRABBED_PLAYERS", "EXECUTE_GRABBED_PLAYERS"}, seen)
        for kind in seen:
            self.assertGreaterEqual(self.cpp.count('"' + kind + '"'), 2)

    def test_saved_flow_reference_resolves_exact_slots_without_hidden_steps(self) -> None:
        physical = json.loads(GAMEPLAY_PATH.read_text(encoding="utf-8"))
        reference = physical["decisionModel"]["scriptedSequence"]
        self.assertEqual({"sequenceId", "mode", "flowId"}, set(reference))
        self.assertEqual(SAVED_DEFAULT_FLOW["flowId"], reference["flowId"])
        self.assertEqual(
            [slot["patternId"] for slot in SAVED_DEFAULT_FLOW["slots"]],
            self.gameplay["decisionModel"]["scriptedSequence"]["patternIds"],
        )
        self.assertIn("Resolve_SavedFlowSequence(", self.cpp)
        self.assertIn("SavedFlowSourceRevision", self.cpp)
        self.assertIn("Compute_SourceRevision", self.cpp)
        self.assertIn("CValtanPatternFlowDocument::Parse_Text", self.cpp)
        self.assertIn("CValtanPatternFlowDocument::Validate", self.cpp)
        self.assertIn("GameplayPath.parent_path().parent_path()", self.cpp)
        self.assertIn("saved Flow path escaped its authoring snapshot", self.cpp)

    def test_first_scripted_entry_owner_matches_publisher_without_weighted_admission(self) -> None:
        joined = tuning_pipeline.join_v2_authoring(
            self.gameplay, self.presentation, self.world_sets,
            self.combat_authoring,
        )
        decision = joined["decisionModel"]
        original_entry_id = "VALTAN_ENTRANCE_CINEMATIC"
        idle_entry_id = "VALTAN_ENTRANCE_CINEMATIC_IDLE"
        entry_ids = (original_entry_id, idle_entry_id)
        owned_ids = {
            row["patternId"]
            for selection_set in decision["selectionSets"]
            for row in selection_set["candidates"]
        } | {
            row["patternId"] for row in decision["mechanics"]
        } | {
            row["patternId"] for row in decision["manualAuditions"]
        }
        for entry_id in entry_ids:
            self.assertNotIn(entry_id, owned_ids)
            entry = next(
                row for row in joined["patterns"]
                if row["patternId"] == entry_id
            )
            product = next(
                row for row in self.encounter["patterns"]
                if row["patternId"] == entry_id
            )
            self.assertEqual("NORMAL", product["selectionMode"])
            self.assertEqual(
                tuning_pipeline.compile_pattern_product(joined, entry), product
            )
        sequence_ids = decision["scriptedSequence"]["patternIds"]
        self.assertEqual(idle_entry_id, sequence_ids[0])
        self.assertEqual(1, sum(row in entry_ids for row in sequence_ids))
        self.assertNotIn(original_entry_id, sequence_ids)

        split = self.cpp[
            self.cpp.index("bool_t Parse_SplitMasterDocument("):
            self.cpp.index("bool_t Apply_MasterDocument(")
        ]
        owner = split[
            split.index("const uint32_t iOwnerCount ="):
            split.index("DATA_JSON_VALUE::OBJECT LegacyPattern;")
        ]
        for token in (
            "Is_OptionalEntryPatternId(strPatternId)",
            "bOptionalEntryPattern && 0u == iOwnerCount",
            "EntryInSequence != ScriptedSequence.PatternIds.begin()",
            "(bOptionalEntryPattern && !bScriptedEntryOnly)",
            "(bCandidate || bScriptedEntryOnly)",
            "ScriptedEntryOnlyPatternIds.insert(strPatternId)",
        ):
            self.assertIn(token, owner)
        for entry_id in entry_ids:
            self.assertIn(entry_id, self.cpp)
        self.assertIn("ScriptedEntryOnlyPatternIds, Out, strOutError", split)
        compatibility = self.cpp[
            self.cpp.index("bool_t Parse_MasterDocument("):
            self.cpp.index("bool_t Read_OptionalOrderedHitOffsets(")
        ]
        self.assertIn(
            "for (const std::string& PatternId : ScriptedEntryOnlyPatternIds)",
            compatibility,
        )
        self.assertIn("ManagedNormalPatternIds.erase(PatternId)", compatibility)
        self.assertIn("WeightedPatternIds.contains(PatternId)", compatibility)
        self.assertIn("WeightedPatternIds != ManagedNormalPatternIds", compatibility)

    def test_scripted_entry_owner_failures_keep_the_staged_load_boundary(self) -> None:
        entry_ids = (
            "VALTAN_ENTRANCE_CINEMATIC",
            "VALTAN_ENTRANCE_CINEMATIC_IDLE",
        )
        invalid_sources: list[tuple[str, dict, str]] = []
        for entry_id in entry_ids:
            later_entry = copy.deepcopy(self.gameplay)
            sequence = later_entry["decisionModel"]["scriptedSequence"]["patternIds"]
            sequence[:] = ["VALTAN_WHIRLWIND", entry_id]
            invalid_sources.append((
                f"{entry_id} is no longer first", later_entry,
                "optional entry cinematic",
            ))
            zero_weight = copy.deepcopy(self.gameplay)
            next(
                row for row in zero_weight["patterns"]
                if row["patternId"] == entry_id
            )["compatibilitySelectionWeight"] = 0
            invalid_sources.append((
                f"{entry_id} has zero compatibility weight", zero_weight,
                "compatibilitySelectionWeight must be positive",
            ))
        mixed_entries = copy.deepcopy(self.gameplay)
        mixed_entries["decisionModel"]["scriptedSequence"]["patternIds"][:] = [
            entry_ids[0], entry_ids[1], "VALTAN_WHIRLWIND",
        ]
        invalid_sources.append((
            "both optional entrances are present", mixed_entries,
            "optional entry cinematic",
        ))
        overlap = copy.deepcopy(self.gameplay)
        candidate_id = overlap["decisionModel"]["selectionSets"][0]["candidates"][0]["patternId"]
        manual = copy.deepcopy(overlap["decisionModel"]["manualAuditions"][0])
        manual["patternId"] = candidate_id
        manual["sourceChainId"] = "fixture.owner-overlap"
        overlap["decisionModel"]["manualAuditions"].append(manual)
        next(row for row in overlap["patterns"] if row["patternId"] == candidate_id)[
            "compatibilitySelectionWeight"
        ] = 0
        invalid_sources.append(("decision owners overlap", overlap, "ownership overlaps"))
        for label, gameplay, error in invalid_sources:
            with self.subTest(label=label), self.assertRaisesRegex(
                tuning_pipeline.PipelineError, error,
            ):
                tuning_pipeline.join_v2_authoring(
                    gameplay, self.presentation, self.world_sets,
                    self.combat_authoring,
                )

        load_body = self.cpp[self.cpp.index(
            "bool_t Client::CValtanPatternTree::Load_FromAuthoringPaths("
        ):]
        commit = "OutView = std::move(Staged);"
        self.assertEqual(1, load_body.count("OutView ="))
        self.assertIn(commit, load_body)
        self.assertLess(load_body.index("Parse_SplitMasterDocument("), load_body.index(commit))
        self.assertLess(load_body.index("Apply_MasterDocument("), load_body.index(commit))

    def test_tree_keeps_none_animation_and_stage_clock_cue_independent(self) -> None:
        fist = next(
            row for row in self.presentation["patterns"]
            if row["patternId"] == "VALTAN_FIST_IN_OUT"
        )
        stage = fist["stages"][0]
        self.assertEqual({"mode": "NONE"}, stage["animation"])
        self.assertEqual([], stage["effectCues"])
        independent = next(row for row in self.presentation["independentEffects"]
                           if row["independentEffectId"] == "valtan.independent-effect.donut-in-out")
        self.assertEqual("SERVER_COMBAT_OBJECT", independent["ownership"])
        self.assertEqual("event.valtan.fist-in-out.spawn-donut", independent["spawnEventId"])
        for token in (
            "bSuppressAnimation",
            "bUsesStageClock",
            "NONE animation cannot own a clip wall budget",
            "split stage-clock cue must be an independent NONE-stage Effect",
            "Valtan stage-clock Effect cue left its NONE stage",
        ):
            self.assertIn(token, self.cpp + self.header)

    def test_optional_source_end_and_typed_scale_policy_fail_closed(self) -> None:
        self.assertIn('pSourceEnd->Is_Null()', self.cpp)
        self.assertIn(
            "(!Product.bHasSourceEnd ||", self.cpp,
            "a natural cue must not compare an unowned numeric source end",
        )
        self.assertIn("constexpr uint32_t FORMAT_VERSION = 4u", self.effect_cue_cpp)
        self.assertIn("Read_ScalePolicy", self.effect_cue_cpp)
        self.assertIn(
            "Managed Valtan pattern Effect cue requires explicit scalePolicy",
            self.effect_cue_cpp,
        )
        self.assertIn("Try_BuildCueScalePolicyAnchor", self.effect_service_cpp)
        self.assertIn("WorldScale.x / fScaleX", self.effect_service_cpp)

    def test_level_audition_reads_rotation_v4_candidates(self) -> None:
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

    def test_optional_gameplay_stage_extensions_survive_the_cpp_join(self) -> None:
        def stage(document: dict, pattern_id: str, stage_id: str) -> dict:
            return next(
                candidate
                for pattern in document["patterns"]
                if pattern["patternId"] == pattern_id
                for candidate in pattern["stages"]
                if candidate["stageId"] == stage_id
            )

        dash = stage(self.gameplay, "VALTAN_DASH_CHARGE", "GROGGY")
        dash_product = stage(
            self.encounter, "VALTAN_DASH_CHARGE", "GROGGY"
        )
        self.assertEqual(
            "DESTROY_FIRST_ELIGIBLE", dash["partDamagePolicy"]
        )
        self.assertEqual(
            dash["partDamagePolicy"], dash_product["partDamagePolicy"]
        )

        trash = stage(self.gameplay, "VALTAN_TRASH", "STEP_07")
        trash_product = stage(self.encounter, "VALTAN_TRASH", "STEP_07")
        expected_proxy = {
            "space": "BOSS_LOCAL",
            "forwardOffsetM": 1.0,
            "rightOffsetM": 0.0,
            "radiusM": 2.25,
        }
        self.assertEqual(expected_proxy, trash["counterProxy"])
        self.assertEqual(trash["counterProxy"], trash_product["counterProxy"])
        self.assertTrue(split_policy_accepts(
            self.gameplay,
            self.presentation,
            self.encounter,
            self.world_sets,
            self.combat_authoring,
            require_product_parity=True,
        ))

        invalid_extensions: list[tuple[str, dict]] = []

        invalid_policy = copy.deepcopy(self.gameplay)
        stage(
            invalid_policy, "VALTAN_DASH_CHARGE", "GROGGY"
        )["partDamagePolicy"] = "DESTROY_ALL"
        invalid_extensions.append(("unsupported part policy", invalid_policy))

        missing_part_branch = copy.deepcopy(self.gameplay)
        stage(
            missing_part_branch, "VALTAN_DASH_CHARGE", "GROGGY"
        )["branches"] = [
            branch
            for branch in stage(
                missing_part_branch, "VALTAN_DASH_CHARGE", "GROGGY"
            )["branches"]
            if branch["outcome"] != "PART_DESTROYED"
        ]
        invalid_extensions.append(("missing part branch", missing_part_branch))

        missing_groggy_close = copy.deepcopy(self.gameplay)
        stage(
            missing_groggy_close, "VALTAN_DASH_CHARGE", "GROGGY"
        )["events"] = [
            event
            for event in stage(
                missing_groggy_close, "VALTAN_DASH_CHARGE", "GROGGY"
            )["events"]
            if event["trigger"] != "EXIT"
        ]
        invalid_extensions.append(("unclosed groggy flag", missing_groggy_close))

        invalid_proxy = copy.deepcopy(self.gameplay)
        stage(invalid_proxy, "VALTAN_TRASH", "STEP_07")["counterProxy"][
            "radiusM"
        ] = 0.0
        invalid_extensions.append(("invalid proxy radius", invalid_proxy))

        missing_counter_branch = copy.deepcopy(self.gameplay)
        stage(
            missing_counter_branch, "VALTAN_TRASH", "STEP_07"
        )["branches"] = [
            branch
            for branch in stage(
                missing_counter_branch, "VALTAN_TRASH", "STEP_07"
            )["branches"]
            if branch["outcome"] != "COUNTER_HIT"
        ]
        invalid_extensions.append(
            ("missing counter branch", missing_counter_branch)
        )

        missing_counter_close = copy.deepcopy(self.gameplay)
        stage(
            missing_counter_close, "VALTAN_TRASH", "STEP_07"
        )["events"] = [
            event
            for event in stage(
                missing_counter_close, "VALTAN_TRASH", "STEP_07"
            )["events"]
            if event["trigger"] != "EXIT"
        ]
        invalid_extensions.append(
            ("unclosed counter flag", missing_counter_close)
        )

        for name, gameplay in invalid_extensions:
            with self.subTest(name=name):
                self.assertFalse(split_policy_accepts(
                    gameplay,
                    self.presentation,
                    self.encounter,
                    self.world_sets,
                    self.combat_authoring,
                    require_product_parity=False,
                ))

        split_shape_start = self.cpp.index(
            "Has_ExactPropertiesWithOptional(GameplayStage"
        )
        split_shape = self.cpp[split_shape_start:split_shape_start + 640]
        self.assertIn('"partDamagePolicy"', split_shape)
        self.assertIn('"counterProxy"', split_shape)
        for token in (
            "Read_StageGameplayExtensions",
            "Validate_SplitGameplayStageExtensions",
            "split gameplay instant part destruction contract is invalid",
            "split gameplay counterProxy preset requires WINDUP",
            'LegacyStage.emplace("partDamagePolicy"',
            'LegacyStage.emplace("counterProxy"',
            "Product.strPartDamagePolicy == Master.strPartDamagePolicy",
            "Equal_CounterProxy(Product.CounterProxy, ProductCounterProxy)",
            "Stage.strPartDamagePolicy = Source.strPartDamagePolicy",
            "Stage.CounterProxy = Source.CounterProxy",
            "struct VALTAN_COUNTER_PROXY_VIEW final",
            'std::string strPartDamagePolicy = "NORMAL"',
            "std::optional<VALTAN_COUNTER_PROXY_VIEW> CounterProxy",
        ):
            self.assertIn(token, self.cpp + self.header)

        load_from_authoring = self.cpp[self.cpp.index(
            "bool_t Client::CValtanPatternTree::Load_FromAuthoringPaths("
        ):]
        self.assertIn(
            "Read_StageGameplayExtensions(StageValue",
            load_from_authoring,
            "the Product encounter parser must retain the same extensions",
        )

    def test_server_motion_uses_the_ordered_entry_stage_not_a_literal_name(
        self,
    ) -> None:
        pattern = next(
            row for row in self.gameplay["patterns"]
            if row["patternId"] == "VALTAN_SIX_PIZZA_106"
        )
        motion = pattern["serverMotion"]
        entry_stage = pattern["stages"][0]
        travel_stage_index = next(
            index for index, stage in enumerate(pattern["stages"])
            if stage["stageId"] == motion["travelStageId"]
        )
        travel_stage = pattern["stages"][travel_stage_index]

        self.assertEqual("STEP_01", entry_stage["stageId"])
        self.assertEqual(pattern["entryActionId"], entry_stage["actionId"])
        self.assertLess(motion["takeoffStartMs"], motion["takeoffEndMs"])
        self.assertLessEqual(
            motion["takeoffEndMs"], entry_stage["durationMs"]
        )
        self.assertGreater(travel_stage_index, 0)
        self.assertLess(motion["travelStartMs"], motion["travelEndMs"])
        self.assertLessEqual(
            motion["travelEndMs"], travel_stage["durationMs"]
        )

        validator_begin = self.cpp.index(
            "Validate_PatternServerMotionStageWindows"
        )
        validator_end = self.cpp.index("struct MASTER_PATTERN", validator_begin)
        validator = self.cpp[validator_begin:validator_end]
        self.assertNotIn('"TAKEOFF" != Stages.front().strStageId', validator)
        self.assertNotIn(
            "takeoffStage.stageId -cne 'TAKEOFF'",
            self.pattern_master_projector,
        )
        self.assertNotIn(
            "serverMotion.travelStageId -ceq 'TAKEOFF'",
            self.pattern_master_projector,
        )
        self.assertIn(
            "entryActionId must own the first ordered serverMotion stage",
            self.pattern_master_projector,
        )
        self.assertIn(
            "serverMotion travel stage must follow the entry stage",
            self.pattern_master_projector,
        )

    def test_phase_two_leaps_use_fast_lift_and_center_terrain_landings(
        self,
    ) -> None:
        patterns = {
            row["patternId"]: row for row in self.gameplay["patterns"]
        }
        expected_world_sets = {
            "VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK":
                "worldeventset.valtan.terrain-destruction-3.floor84",
            "VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK":
                "worldeventset.valtan.terrain-destruction-9.floor30",
        }

        for pattern_id in (
            "VALTAN_SIX_PIZZA_106",
            *expected_world_sets,
        ):
            motion = patterns[pattern_id]["serverMotion"]
            self.assertEqual(800, motion["takeoffStartMs"])
            self.assertEqual(1100, motion["takeoffEndMs"])

        pizza = patterns["VALTAN_SIX_PIZZA_106"]
        pizza_motion = pizza["serverMotion"]
        axe_motion = patterns["VALTAN_HIGH_JUMP"]["serverMotion"]
        self.assertEqual(
            axe_motion["travelEndMs"] - axe_motion["travelStartMs"],
            pizza_motion["travelEndMs"] - pizza_motion["travelStartMs"],
        )
        self.assertEqual(10.0, pizza_motion["apexHeight"])
        self.assertEqual("STEP_03", pizza_motion["travelStageId"])
        self.assertEqual(1200, pizza["stages"][2]["durationMs"])
        self.assertEqual(
            [1200, 1000, 1200],
            [row["durationMs"] for row in pizza["stages"][6:9]],
        )

        for pattern_id, expected_world_set in expected_world_sets.items():
            pattern = patterns[pattern_id]
            self.assertEqual(
                [156.03, 22.99751, -122.06],
                pattern["serverMotion"]["landingPosition"],
            )
            impact = next(
                stage for stage in pattern["stages"]
                if stage["stageId"] == "IMPACT"
            )
            world_events = [
                event["worldEventSetId"] for event in impact["events"]
                if event["kind"] == "TRIGGER_WORLD_EVENT_SET"
            ]
            self.assertEqual([expected_world_set], world_events)

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
        orphaned_cue_owner["independentEffects"].append({
            "independentEffectId": "valtan.independent-effect.missing",
            "displayName": "orphaned stage cue", "ownership": "SERVER_PATTERN_STAGE",
            "cueId": "cue.valtan.missing",
        })
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

    def test_animation_chain_promotions_are_manual_only_product_patterns(self) -> None:
        audition_rows = self.gameplay["decisionModel"]["manualAuditions"]
        manual_rows = [
            row for row in audition_rows
            if row["admissionState"] == "MANUAL_SERVER_AUDITION"
        ]
        derived_rows = [
            row for row in audition_rows
            if row["admissionState"] == "DERIVED_SERVER_PATTERN"
        ]
        mechanic_ids = {
            row["patternId"]
            for row in self.gameplay["decisionModel"]["mechanics"]
        }
        manifest_rows = self.promotion_manifest["patterns"]
        manifest_ids = [row["patternId"] for row in manifest_rows]
        lineage = lambda row: (
            row["patternId"], row["sourceChainId"],
            row["authoringPhase"], row["admissionState"],
        )
        self.assertEqual(len(audition_rows), len(manual_rows) + len(derived_rows))
        self.assertEqual(
            len(audition_rows), len({row["patternId"] for row in audition_rows})
        )
        self.assertEqual(
            len(audition_rows), len({row["sourceChainId"] for row in audition_rows})
        )
        for row in audition_rows:
            self.assertTrue(STABLE_TOKEN.fullmatch(row["patternId"]))
            self.assertTrue(STABLE_TOKEN.fullmatch(row["sourceChainId"]))
        self.assertEqual(
            [lineage(row) for row in manual_rows],
            [lineage(row) for row in manifest_rows],
        )
        self.assertIn("VALTAN_GHOST_FINALE", [row["patternId"] for row in derived_rows])
        self.assertTrue(
            {
                "VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK",
                "VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK",
            }.issubset(mechanic_ids)
        )
        gameplay_names = {
            row["patternId"]: row["displayName"]
            for row in self.gameplay["patterns"]
        }
        self.assertEqual(len(manifest_ids), len(set(manifest_ids)))
        for row in manifest_rows:
            self.assertEqual(gameplay_names[row["patternId"]], row["displayName"])

        products = {
            row["patternId"]: row for row in self.encounter["patterns"]
        }
        retired_id = "VALTAN_SEQUENCE_FRONT_BACK_FRONT"
        self.assertIn(retired_id, self.gameplay["retiredPatternIds"])
        self.assertNotIn(retired_id, products)
        self.assertTrue({"VALTAN_FRONT_BACK_FRONT", "VALTAN_GHOST_TRANSITION_15"}.issubset(products))
        for rows in (self.gameplay["patterns"], manifest_rows, self.encounter["patterns"]):
            four = next(row for row in rows if row["patternId"] == "VALTAN_SEQUENCE_FOUR")
            self.assertEqual("2페이즈 4방향 공격", four["displayName"])
        joined = tuning_pipeline.join_v2_authoring(
            self.gameplay, self.presentation,
            self.world_sets, self.combat_authoring,
        )
        authored = {
            row["patternId"]: row for row in joined["patterns"]
        }
        enriched_stage_count = 0
        for manual in manual_rows:
            product = products[manual["patternId"]]
            self.assertEqual("AUDITION_ONLY", product["selectionMode"])
            self.assertNotEqual("MECHANIC", product["category"])
            self.assertEqual(
                (0, 0, 0, 0, 0, 0),
                (
                    product["minimumHealthBar"],
                    product["maximumHealthBar"],
                    product["triggerHealthBar"],
                    product["triggerOrder"],
                    product["selectionWeight"],
                    product["maximumConsecutiveUses"],
                ),
            )
            self.assertTrue(product["stages"])
            self.assertEqual(
                tuning_pipeline.compile_pattern_product(
                    joined, authored[manual["patternId"]]
                ),
                product,
            )
            enriched_stage_count += sum(
                stage["hitShape"] != "NONE"
                or "motion" in stage
                or "actions" in stage
                or "branches" in stage
                for stage in product["stages"]
            )
        if manual_rows:
            self.assertGreater(enriched_stage_count, 0)

        self.assertEqual(
            3,
            next(row["authoringPhase"] for row in manifest_rows if row["patternId"] == "VALTAN_STRUGGLING"),
        )
        for token in (
            "VALTAN_MANUAL_AUDITION_VIEW",
            '"manualAuditions"',
            '"AUDITION_ONLY"',
            "bManualServerAudition",
            "strSourceAnimationChainId",
        ):
            self.assertIn(token, self.cpp + self.header)
        for token in (
            '"Manual Audition | P"',
            '"Animation-first manual audition | phase %u | source chain %s | automatic rotation disabled"',
        ):
            self.assertIn(token, self.effect_tool_cpp)

    def test_counter_reaction_and_weighted_selection_are_exact_product_joins(self) -> None:
        self.assertEqual(
            EXPECTED_SCRIPTED_SEQUENCE,
            self.gameplay["decisionModel"]["scriptedSequence"],
        )
        self.assertEqual(
            EXPECTED_SCRIPTED_SEQUENCE,
            self.rotations["scriptedSequence"],
        )
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

        product_mutations: list[tuple[str, dict]] = []
        old_version = copy.deepcopy(self.rotations)
        old_version["formatVersion"] = 3
        product_mutations.append(("old Product version", old_version))

        missing_sequence = copy.deepcopy(self.rotations)
        missing_sequence.pop("scriptedSequence")
        product_mutations.append(("missing sequence", missing_sequence))

        reordered = copy.deepcopy(self.rotations)
        reordered["scriptedSequence"]["patternIds"][0:2] = reversed(
            reordered["scriptedSequence"]["patternIds"][0:2]
        )
        product_mutations.append(("sequence order drift", reordered))

        repeated = copy.deepcopy(self.rotations)
        repeated["scriptedSequence"]["mode"] = "ORDERED_REPEAT"
        product_mutations.append(("unsupported sequence mode", repeated))

        invalid_pursuit = copy.deepcopy(self.rotations)
        invalid_pursuit["scriptedSequence"]["interStepPursuitMs"] = 0
        product_mutations.append(("invalid inter-step pursuit", invalid_pursuit))

        pursuit_drift = copy.deepcopy(self.rotations)
        pursuit_drift["scriptedSequence"]["interStepPursuitMs"] = 900
        product_mutations.append(("inter-step pursuit parity drift", pursuit_drift))

        unexpected_field = copy.deepcopy(self.rotations)
        unexpected_field["scriptedSequence"]["unexpectedField"] = True
        product_mutations.append(("unexpected sequence field", unexpected_field))

        for name, rotations in product_mutations:
            with self.subTest(name=name):
                self.assertFalse(shared_reaction_and_selection_contract_valid(
                    self.master, self.gameplay, self.encounter, self.bindings,
                    rotations,
                ))

        authored_order_drift = copy.deepcopy(self.gameplay)
        authored_order_drift["decisionModel"]["scriptedSequence"][
            "patternIds"
        ][0:2] = reversed(
            authored_order_drift["decisionModel"]["scriptedSequence"][
                "patternIds"
            ][0:2]
        )
        self.assertFalse(shared_reaction_and_selection_contract_valid(
            self.master, authored_order_drift, self.encounter, self.bindings,
            self.rotations,
        ))

        selection_mode_agnostic = copy.deepcopy(self.encounter)
        scripted_pattern_ids = set(EXPECTED_SCRIPTED_SEQUENCE["patternIds"])
        for pattern in selection_mode_agnostic["patterns"]:
            if pattern["patternId"] in scripted_pattern_ids:
                pattern["selectionMode"] = "AUDITION_ONLY"
        self.assertTrue(shared_reaction_and_selection_contract_valid(
            self.master, self.gameplay, selection_mode_agnostic, self.bindings,
            self.rotations,
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
            "MASTER_SCRIPTED_SEQUENCE_VIEW",
            "Valtan scripted-sequence Product parity drifted",
        ):
            self.assertIn(token, self.cpp + self.header)
        self.assertNotIn(
            "scriptedSequence pattern has no automatic decision owner",
            self.cpp,
        )

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

    def test_v4_action_validator_admits_authored_combat_objects_and_phase_transition(
        self,
    ) -> None:
        source_events = [
            event
            for pattern in self.gameplay["patterns"]
            for stage in pattern["stages"]
            for event in stage.get("events", [])
        ]
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
        retarget_sources = [
            event for event in source_events
            if event.get("kind") == "RETARGET_RANDOM_ALIVE"
        ]
        release_sources = [
            event for event in source_events
            if event.get("kind") == "RELEASE_GRABBED_PLAYERS"
        ]
        retargets = [
            action for action in actions
            if action.get("kind") == "RETARGET_RANDOM_ALIVE"
        ]
        releases = [
            action for action in actions
            if action.get("kind") == "RELEASE_GRABBED_PLAYERS"
        ]
        self.assertTrue(volleys)
        self.assertEqual(1, len(phase_changes))
        self.assertEqual(19, len(retarget_sources))
        self.assertEqual(3, len(release_sources))
        self.assertEqual(len(retarget_sources), len(retargets))
        self.assertEqual(len(release_sources), len(releases))
        for event in retarget_sources:
            self.assertEqual(
                {"eventId", "trigger", "kind"}, set(event)
            )
            self.assertEqual("ENTER", event["trigger"])
        for action in retargets:
            self.assertEqual({
                "trigger", "kind", "targetId", "value", "durationMs",
            }, set(action))
            self.assertEqual("ENTER", action["trigger"])
            self.assertEqual("boss.target.pattern", action["targetId"])
            self.assertEqual(1, action["value"])
            self.assertEqual(0, action["durationMs"])
        for event in release_sources:
            self.assertEqual({
                "eventId", "trigger", "kind", "releaseMode",
                "speedMps", "durationMs", "yawOffsetDegrees",
            }, set(event))
        for action in releases:
            self.assertEqual({
                "trigger", "kind", "targetId", "releaseMode",
                "speedMps", "durationMs", "yawOffsetDegrees",
            }, set(action))
            self.assertEqual("boss.attachment.left-hand", action["targetId"])
            self.assertGreaterEqual(action["yawOffsetDegrees"], -180)
            self.assertLessEqual(action["yawOffsetDegrees"], 180)
            self.assertTrue(
                action["releaseMode"] == "HOLD"
                and action["speedMps"] == 0
                and action["durationMs"] == 0
                and action["yawOffsetDegrees"] == 0
                or action["releaseMode"] in ("OPPOSITE_KNOCKBACK", "ARENA_EJECTION")
                and 0 < action["speedMps"] <= 50
                and 0 < action["durationMs"] <= 5000
                and (
                    action["releaseMode"] == "ARENA_EJECTION"
                    or action["yawOffsetDegrees"] == 0
                )
            )

        release_field_contract = (
            '"speedMps", "durationMs", "yawOffsetDegrees"'
        )
        self.assertGreaterEqual(self.cpp.count(release_field_contract), 3)
        self.assertIn(release_field_contract, self.encounter_reference_cpp)
        self.assertIn(
            'std::abs(yawOffsetDegrees) > 180.0',
            self.encounter_reference_cpp,
        )
        self.assertIn(
            '"master grabbed-player release action is incoherent"', self.cpp
        )
        self.assertIn('"boss.attachment.left-hand" != strTargetId', self.cpp)
        for volley in volleys:
            self.assertEqual({
                "trigger", "kind", "targetId", "targetingPolicy",
                "countPerResolvedTarget", "layout", "radiusM",
                "startAngleDegrees", "angleStepDegrees", "allowOverlap",
                "maximumTotalObjects", "spawnCount", "spawnIntervalMs",
                "arenaRandomCount", "arenaRandomRadiusM",
                "arenaHeightToleranceM", "arenaAnchorPolicy",
            }, set(volley))
            self.assertEqual("ENTER", volley["trigger"])
            self.assertEqual("PER_ALIVE_PLAYER", volley["targetingPolicy"])
            self.assertEqual(3, volley["spawnCount"])
            self.assertEqual(1333, volley["spawnIntervalMs"])
            self.assertEqual(4, volley["arenaRandomCount"])
            self.assertEqual(14.0, volley["arenaRandomRadiusM"])
            self.assertEqual(1.0, volley["arenaHeightToleranceM"])
            self.assertEqual(
                "BOSS_SPAWN_POSITION", volley["arenaAnchorPolicy"]
            )
            self.assertEqual(36, volley["maximumTotalObjects"])
            self.assertGreaterEqual(
                volley["maximumTotalObjects"],
                volley["countPerResolvedTarget"] + volley["arenaRandomCount"],
            )

        axe_source = next(
            event
            for pattern in self.gameplay["patterns"]
            if pattern["patternId"] == "VALTAN_HIGH_JUMP"
            for stage in pattern["stages"]
            if stage["stageId"] == "AIRBORNE"
            for event in stage["events"]
            if event["eventId"]
            == "event.valtan.high-jump.airborne.spawn-target-axe"
        )
        self.assertEqual({
            "eventId", "trigger", "kind", "combatObjectArchetypeId",
            "volleyPolicy", "countPerResolvedTarget", "layout",
            "spawnSchedule", "arenaRandom", "allowOverlap",
            "maximumTotalObjects",
        }, set(axe_source))
        self.assertEqual(
            {"kind": "INTERVAL", "count": 3, "firstOffsetMs": 0,
             "intervalMs": 1333},
            axe_source["spawnSchedule"],
        )
        self.assertEqual(
            {"kind": "RANDOM_NAVIGABLE_CIRCLE",
             "anchor": "BOSS_SPAWN_POSITION", "count": 4,
             "radiusM": 14.0, "heightToleranceM": 1.0},
            axe_source["arenaRandom"],
        )
        self.assertEqual({
            "trigger": "ENTER",
            "kind": "SET_GAMEPLAY_PHASE",
            "targetId": "boss.phase.gameplay",
            "value": 2,
            "durationMs": 0,
        }, phase_changes[0])

        invalid_events: list[tuple[str, dict]] = []
        retarget_exit = copy.deepcopy(self.gameplay)
        next(
            event
            for pattern in retarget_exit["patterns"]
            for stage in pattern["stages"]
            for event in stage.get("events", [])
            if event.get("kind") == "RETARGET_RANDOM_ALIVE"
        )["trigger"] = "EXIT"
        invalid_events.append(("retarget on EXIT", retarget_exit))

        retarget_extra = copy.deepcopy(self.gameplay)
        next(
            event
            for pattern in retarget_extra["patterns"]
            for stage in pattern["stages"]
            for event in stage.get("events", [])
            if event.get("kind") == "RETARGET_RANDOM_ALIVE"
        )["unexpected"] = True
        invalid_events.append(("retarget extra field", retarget_extra))

        release_invalid = copy.deepcopy(self.gameplay)
        invalid_release = next(
            event
            for pattern in release_invalid["patterns"]
            for stage in pattern["stages"]
            for event in stage.get("events", [])
            if event.get("kind") == "RELEASE_GRABBED_PLAYERS"
            and event["releaseMode"] == "HOLD"
        )
        invalid_release["speedMps"] = 1.0
        invalid_events.append(("invalid HOLD release", release_invalid))

        for name, gameplay in invalid_events:
            with self.subTest(name=name):
                self.assertFalse(split_policy_accepts(
                    gameplay, self.presentation, self.encounter,
                    self.world_sets, self.combat_authoring,
                    require_product_parity=False,
                ))
        for marker in (
            'actionKind->Get_String() == "SPAWN_COMBAT_OBJECT_VOLLEY"',
            'kind == "SET_GAMEPLAY_PHASE"',
            'kind == "RETARGET_RANDOM_ALIVE"',
            'actionKind->Get_String() == "RELEASE_GRABBED_PLAYERS"',
            'targetingPolicy != "PER_ALIVE_PLAYER"',
            'targetId == "boss.phase.gameplay" && 2u == value',
            'targetId == "boss.target.pattern" && 1u == value',
            'targetId != "boss.attachment.left-hand"',
            'else if ("SPAWN_COMBAT_OBJECT" == strKind)',
            '"RETARGET_RANDOM_ALIVE" == strKind',
            'else if ("RELEASE_GRABBED_PLAYERS" == strKind)',
            '"split gameplay grabbed-player release event is invalid"',
            '"split gameplay combat-object spawn is invalid"',
            '"spawnSchedule"',
            '"arenaRandom"',
            '"arenaRandomCount", "arenaRandomRadiusM"',
            '"RANDOM_NAVIGABLE_CIRCLE"',
            '"BOSS_SPAWN_POSITION"',
        ):
            self.assertIn(
                marker,
                self.encounter_reference_cpp + self.cpp + self.balance_tool_cpp,
            )

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
