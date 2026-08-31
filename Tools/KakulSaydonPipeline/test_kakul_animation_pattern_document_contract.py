import copy
import json
import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
REFERENCE_ROOT = ROOT / "Data" / "Animation" / "Reference" / "KakulSaydon"
AUTHORED_ROOT = ROOT / "Data" / "Animation" / "Authored" / "KakulSaydon"
HEADER = ROOT / "Client" / "Public" / "KakulAnimationPatternDocument.h"
SOURCE = ROOT / "Client" / "Private" / "KakulAnimationPatternDocument.cpp"
ANIMATION_TOOL_SOURCE = ROOT / "Client" / "Private" / "Animation_Tool.cpp"
PROJECT = ROOT / "Client" / "Default" / "Client.vcxproj"
FILTERS = ROOT / "Client" / "Default" / "Client.vcxproj.filters"

PROFILES = ("MN_RPCT_05", "MN_RPCT_06", "MN_RPCT_07", "MN_RPCZ_00")
ROOT_FIELDS = {
    "schema",
    "formatVersion",
    "profileId",
    "referenceRevision",
    "authority",
    "nextPatternOrdinal",
    "patterns",
}
PATTERN_FIELDS = {
    "patternId",
    "displayName",
    "sourceActionId",
    "nextOccurrenceOrdinal",
    "clips",
}
CLIP_FIELDS = {
    "occurrenceId",
    "stageId",
    "slotId",
    "runtimeClip",
    "sourceStartMs",
    "playMs",
    "playRate",
    "endPolicy",
}
END_POLICIES = {"EXACT", "HOLD_LAST_POSE", "LOOP_TO_WINDOW"}


def _reference_slots(reference):
    slots = set()
    runtime_clips = set()
    action_ids = set()
    for action in reference["actions"]:
        is_review_candidate = action["reviewStatus"] == "REVIEW_CANDIDATE"
        if is_review_candidate:
            action_ids.add(action["sourceActionId"])
        for stage in action["stages"]:
            for slot in stage["slots"]:
                if is_review_candidate:
                    slots.add(
                        (
                            action["sourceActionId"],
                            stage["stageId"],
                            slot["slotId"],
                        )
                    )
                runtime_clips.add(slot["runtimeClip"])
    return action_ids, slots, runtime_clips


def _first_reference_slot(reference):
    for action in reference["actions"]:
        if action["reviewStatus"] != "REVIEW_CANDIDATE":
            continue
        for stage in action["stages"]:
            for slot in stage["slots"]:
                return action, stage, slot
    raise AssertionError("reference has no source slot")


def _first_holdout_reference_slot(reference):
    for action in reference["actions"]:
        if action["reviewStatus"] != "HOLDOUT":
            continue
        for stage in action["stages"]:
            for slot in stage["slots"]:
                return action, stage, slot
    raise AssertionError("reference has no HOLDOUT source slot")


def _validate_contract(document, reference, available_clips, expected_profile, file_name):
    if set(document) != ROOT_FIELDS:
        return False
    if document["schema"] != "lostark.kakul-animation-pattern-bindings":
        return False
    if document["formatVersion"] != 1:
        return False
    if expected_profile not in PROFILES or document["profileId"] != expected_profile:
        return False
    if file_name != f"{expected_profile}.patternbindings.json":
        return False
    if document["referenceRevision"] != reference["referenceRevision"]:
        return False
    if document["authority"] != "REFERENCE_ONLY":
        return False
    next_pattern = document["nextPatternOrdinal"]
    if type(next_pattern) is not int or not 1 <= next_pattern <= 1_000_000:
        return False
    if len(document["patterns"]) > 4096:
        return False

    action_ids, source_slots, _ = _reference_slots(reference)
    pattern_ids = set()
    occurrence_ids = set()
    total_clips = 0
    pattern_shape = re.compile(
        rf"kakul\.{re.escape(expected_profile)}\.pattern\.([1-9][0-9]*)"
    )
    for pattern in document["patterns"]:
        if set(pattern) != PATTERN_FIELDS:
            return False
        match = pattern_shape.fullmatch(pattern["patternId"])
        if not match or int(match.group(1)) >= next_pattern:
            return False
        if pattern["patternId"] in pattern_ids:
            return False
        pattern_ids.add(pattern["patternId"])
        if not pattern["displayName"] or pattern["sourceActionId"] not in action_ids:
            return False
        next_occurrence = pattern["nextOccurrenceOrdinal"]
        if type(next_occurrence) is not int or not 1 <= next_occurrence <= 1_000_000:
            return False
        if not pattern["clips"] or len(pattern["clips"]) > 4096:
            return False
        total_clips += len(pattern["clips"])
        if total_clips > 16384:
            return False

        occurrence_shape = re.compile(
            rf"{re.escape(pattern['patternId'])}\.clip\.([1-9][0-9]*)"
        )
        for clip in pattern["clips"]:
            if set(clip) != CLIP_FIELDS:
                return False
            match = occurrence_shape.fullmatch(clip["occurrenceId"])
            if not match or int(match.group(1)) >= next_occurrence:
                return False
            if clip["occurrenceId"] in occurrence_ids:
                return False
            occurrence_ids.add(clip["occurrenceId"])
            source_key = (
                pattern["sourceActionId"],
                clip["stageId"],
                clip["slotId"],
            )
            if source_key not in source_slots:
                return False
            if clip["runtimeClip"] not in available_clips:
                return False
            if type(clip["sourceStartMs"]) is not int:
                return False
            if not 0 <= clip["sourceStartMs"] <= 600_000:
                return False
            if type(clip["playMs"]) is not int or not 1 <= clip["playMs"] <= 600_000:
                return False
            if isinstance(clip["playRate"], bool):
                return False
            if not 0.01 <= clip["playRate"] <= 16.0:
                return False
            if clip["endPolicy"] not in END_POLICIES:
                return False
    return True


class KakulAnimationPatternDocumentContractTests(unittest.TestCase):
    def _nonempty_document(self, profile="MN_RPCT_05"):
        reference = json.loads(
            (REFERENCE_ROOT / f"{profile}.actionreference.json").read_text(
                encoding="utf-8"
            )
        )
        action, stage, slot = _first_reference_slot(reference)
        pattern_id = f"kakul.{profile}.pattern.1"
        document = {
            "schema": "lostark.kakul-animation-pattern-bindings",
            "formatVersion": 1,
            "profileId": profile,
            "referenceRevision": reference["referenceRevision"],
            "authority": "REFERENCE_ONLY",
            "nextPatternOrdinal": 2,
            "patterns": [
                {
                    "patternId": pattern_id,
                    "displayName": action["displayName"],
                    "sourceActionId": action["sourceActionId"],
                    "nextOccurrenceOrdinal": 2,
                    "clips": [
                        {
                            "occurrenceId": f"{pattern_id}.clip.1",
                            "stageId": stage["stageId"],
                            "slotId": slot["slotId"],
                            "runtimeClip": slot["runtimeClip"],
                            "sourceStartMs": slot["sourceStartMs"],
                            "playMs": slot["playMs"],
                            "playRate": slot["playRate"],
                            "endPolicy": "EXACT",
                        }
                    ],
                }
            ],
        }
        return reference, document

    def test_empty_documents_are_strict_and_pinned_to_current_references(self):
        for profile in PROFILES:
            reference = json.loads(
                (REFERENCE_ROOT / f"{profile}.actionreference.json").read_text(
                    encoding="utf-8"
                )
            )
            path = AUTHORED_ROOT / f"{profile}.patternbindings.json"
            self.assertTrue(path.is_file(), path)
            document = json.loads(path.read_text(encoding="utf-8"))
            self.assertEqual(set(document), ROOT_FIELDS)
            self.assertEqual(
                document["schema"], "lostark.kakul-animation-pattern-bindings"
            )
            self.assertEqual(document["formatVersion"], 1)
            self.assertEqual(document["profileId"], profile)
            self.assertEqual(
                document["referenceRevision"], reference["referenceRevision"]
            )
            self.assertEqual(document["authority"], "REFERENCE_ONLY")
            self.assertEqual(document["nextPatternOrdinal"], 1)
            self.assertEqual(document["patterns"], [])

    def test_contract_rejects_stale_identity_duplicates_ranges_and_paths(self):
        reference, baseline = self._nonempty_document()
        _, _, available_clips = _reference_slots(reference)
        expected_file = "MN_RPCT_05.patternbindings.json"
        self.assertTrue(
            _validate_contract(
                baseline, reference, available_clips, "MN_RPCT_05", expected_file
            )
        )

        mutations = {}
        mutations["extra field"] = copy.deepcopy(baseline)
        mutations["extra field"]["unexpected"] = True
        mutations["version"] = copy.deepcopy(baseline)
        mutations["version"]["formatVersion"] = 2
        mutations["profile"] = copy.deepcopy(baseline)
        mutations["profile"]["profileId"] = "MN_RPCT_06"
        mutations["revision"] = copy.deepcopy(baseline)
        mutations["revision"]["referenceRevision"] = "0" * 64
        mutations["authority"] = copy.deepcopy(baseline)
        mutations["authority"]["authority"] = "PRODUCT"
        mutations["pattern counter reuse"] = copy.deepcopy(baseline)
        mutations["pattern counter reuse"]["nextPatternOrdinal"] = 1
        mutations["occurrence counter reuse"] = copy.deepcopy(baseline)
        mutations["occurrence counter reuse"]["patterns"][0][
            "nextOccurrenceOrdinal"
        ] = 1
        mutations["duplicate pattern"] = copy.deepcopy(baseline)
        mutations["duplicate pattern"]["patterns"].append(
            copy.deepcopy(mutations["duplicate pattern"]["patterns"][0])
        )
        mutations["duplicate occurrence"] = copy.deepcopy(baseline)
        mutations["duplicate occurrence"]["patterns"][0]["clips"].append(
            copy.deepcopy(
                mutations["duplicate occurrence"]["patterns"][0]["clips"][0]
            )
        )
        mutations["source tuple"] = copy.deepcopy(baseline)
        mutations["source tuple"]["patterns"][0]["clips"][0]["slotId"] = "missing"
        mutations["model clip"] = copy.deepcopy(baseline)
        mutations["model clip"]["patterns"][0]["clips"][0][
            "runtimeClip"
        ] = "missing"
        mutations["play range"] = copy.deepcopy(baseline)
        mutations["play range"]["patterns"][0]["clips"][0]["playMs"] = 0
        mutations["play rate"] = copy.deepcopy(baseline)
        mutations["play rate"]["patterns"][0]["clips"][0]["playRate"] = 16.01
        mutations["end policy"] = copy.deepcopy(baseline)
        mutations["end policy"]["patterns"][0]["clips"][0][
            "endPolicy"
        ] = "LOOP"

        for label, document in mutations.items():
            with self.subTest(label=label):
                self.assertFalse(
                    _validate_contract(
                        document,
                        reference,
                        available_clips,
                        "MN_RPCT_05",
                        expected_file,
                    )
                )
        self.assertFalse(
            _validate_contract(
                baseline,
                reference,
                available_clips,
                "MN_RPCT_05",
                "MN_RPCT_06.patternbindings.json",
            )
        )

    def test_holdout_action_is_rejected_even_when_its_source_clip_exists(self):
        reference, document = self._nonempty_document()
        action, stage, slot = _first_holdout_reference_slot(reference)
        self.assertEqual(action["reviewStatus"], "HOLDOUT")
        pattern = document["patterns"][0]
        pattern["displayName"] = action["displayName"]
        pattern["sourceActionId"] = action["sourceActionId"]
        clip = pattern["clips"][0]
        clip["stageId"] = stage["stageId"]
        clip["slotId"] = slot["slotId"]
        clip["runtimeClip"] = slot["runtimeClip"]
        clip["sourceStartMs"] = slot["sourceStartMs"]
        clip["playMs"] = slot["playMs"]
        clip["playRate"] = slot["playRate"]
        _, _, available_clips = _reference_slots(reference)
        self.assertIn(slot["runtimeClip"], available_clips)
        self.assertFalse(
            _validate_contract(
                document,
                reference,
                available_clips,
                "MN_RPCT_05",
                "MN_RPCT_05.patternbindings.json",
            )
        )

    def test_public_boundary_uses_predictable_members_and_transactions(self):
        header = HEADER.read_text(encoding="utf-8")
        for token in (
            "KAKUL_ANIMATION_PATTERN_CLIP",
            "strOccurrenceId",
            "strStageId",
            "strSlotId",
            "strRuntimeClip",
            "iSourceStartMs",
            "iPlayMs",
            "fPlayRate",
            "strEndPolicy",
            "KAKUL_ANIMATION_PATTERN",
            "strPatternId",
            "strDisplayName",
            "iSourceActionId",
            "iNextOccurrenceOrdinal",
            "std::vector<KAKUL_ANIMATION_PATTERN_CLIP> Clips",
            "KAKUL_ANIMATION_PATTERN_DOCUMENT",
            "iFormatVersion",
            "strProfileId",
            "strReferenceRevision",
            "strAuthority",
            "iNextPatternOrdinal",
            "std::vector<KAKUL_ANIMATION_PATTERN> Patterns",
            "Resolve_Path",
            "Parse_Text",
            "Validate",
            "Load_FromPath",
            "Load",
            "Save_Atomic",
        ):
            self.assertIn(token, header)
        self.assertNotIn("bLoop", header)

    def test_cpp_is_strict_reference_only_and_exactly_joined(self):
        source = SOURCE.read_text(encoding="utf-8")
        for token in (
            '"lostark.kakul-animation-pattern-bindings"',
            '"REFERENCE_ONLY"',
            '"EXACT"',
            '"HOLD_LAST_POSE"',
            '"LOOP_TO_WINDOW"',
            "Has_ExactProperties",
            "Build_SourceKey",
            'return "kakul." +',
            '".pattern."',
            '".clip."',
            "patternOrdinal >= document.iNextPatternOrdinal",
            "occurrenceOrdinal >= pattern.iNextOccurrenceOrdinal",
            "referenceSlots.contains(key)",
            'action.strReviewStatus != "REVIEW_CANDIDATE"',
            "reviewCandidateActions.contains(pattern.iSourceActionId)",
            "availableClipSet.contains(clip.strRuntimeClip)",
            "document.strReferenceRevision != reference.strReferenceRevision",
            "Is_ExpectedPath",
            'component == L".."',
        ):
            self.assertIn(token, source)
        self.assertNotRegex(source, r'"loop"')
        for forbidden in ("Data/Valtan", "CValtan"):
            self.assertNotIn(forbidden, source)

    def test_load_stages_before_commit_and_save_is_durable_cas_atomic(self):
        source = SOURCE.read_text(encoding="utf-8")
        for token in (
            "KAKUL_ANIMATION_PATTERN_DOCUMENT staged",
            "outDocument = std::move(staged)",
            "_commit(_fileno(file))",
            "Parse_Text(verificationText, reparsed",
            "Validate(reparsed, currentReference",
            "reparsed != document",
            "finalReference != currentReference",
            "MoveFileExW",
            "MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH",
        ):
            self.assertIn(token, source)
        self.assertLess(
            source.index("_commit(_fileno(file))"), source.index("reparsed != document")
        )
        self.assertLess(
            source.index("reparsed != document"),
            source.index("finalReference != currentReference"),
        )
        self.assertLess(
            source.index("finalReference != currentReference"),
            source.rindex("MoveFileExW"),
        )

    def test_project_and_data_filters_register_the_new_contract(self):
        project = PROJECT.read_text(encoding="utf-8")
        filters = FILTERS.read_text(encoding="utf-8")
        entries = (
            "KakulAnimationPatternDocument.h",
            "KakulAnimationPatternDocument.cpp",
            "MN_RPCT_05.patternbindings.json",
            "MN_RPCT_06.patternbindings.json",
            "MN_RPCT_07.patternbindings.json",
            "MN_RPCZ_00.patternbindings.json",
        )
        for entry in entries:
            self.assertEqual(project.count(entry), 1, entry)
            self.assertEqual(filters.count(entry), 1, entry)
        self.assertIn("96.DataFiles\\Animation\\Authored\\KakulSaydon", filters)

    def test_animation_tool_exposes_all_profiles_and_pattern_workflow(self):
        source = ANIMATION_TOOL_SOURCE.read_text(encoding="utf-8")
        for token in (
            "KAKUL_ACTION_PROFILES",
            '"MN_RPCT_05"',
            '"MN_RPCT_06"',
            '"MN_RPCT_07"',
            '"MN_RPCZ_00"',
            "Render_KakulPatternAuthoring",
            'ImGui::Button("Preview Action")',
            'ImGui::Button("Create Pattern")',
            'ImGui::Button("Play Pattern")',
            'ImGui::Button("Duplicate Pattern")',
            'ImGui::Button("Delete Pattern")',
            'ImGui::Button("Duplicate Clip")',
            'ImGui::Button("Delete Clip")',
            'ImGui::Button("Move Clip Up")',
            'ImGui::Button("Move Clip Down")',
            "m_iKakulPatternPreviewTargetGeneration",
            "CKakulAnimationPatternDocument::Save_Atomic",
            '"REVIEW_CANDIDATE" == pSelectedAction->strReviewStatus',
        ):
            self.assertIn(token, source)


if __name__ == "__main__":
    unittest.main()
