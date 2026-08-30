#!/usr/bin/env python3
"""Focused data contract for Valtan pattern SOUND event bindings.

This keeps the exact authored-event -> animation occurrence -> encounter stage
join executable without linking the Effect authoring implementation into a
broad native Client harness.
"""

from __future__ import annotations

import copy
import json
import re
import unittest

from Tools.ValtanPipeline import build_valtan_pattern_sound_cues as builder


STABLE_ID = re.compile(r"^[A-Za-z0-9_.-]{1,160}$")
ROOT_FIELDS = {"schema", "formatVersion", "ownerArchetypeId", "cues"}
CUE_FIELDS = {
    "bindingId",
    "occurrenceId",
    "patternId",
    "stageId",
    "actionId",
    "clipOccurrenceId",
    "soundBank",
    "soundEvent",
    "repeatPolicy",
    "startMs",
}


def validate_document(document: dict, pattern_bindings: dict, encounter: dict) -> None:
    if not isinstance(document, dict) or set(document) != ROOT_FIELDS:
        raise ValueError("sound document root fields are invalid")
    if document["schema"] != "lostark.valtan-pattern-sound-cues":
        raise ValueError("sound document schema is invalid")
    if document["formatVersion"] != 1 or document["ownerArchetypeId"] != "BOSS_VALTAN":
        raise ValueError("sound document identity/version is invalid")
    if not isinstance(document["cues"], list):
        raise ValueError("sound document cues must be an array")

    actions: dict[str, tuple[str, str]] = {}
    for pattern in encounter.get("patterns", []):
        for stage in pattern.get("stages", []):
            action_id = stage.get("actionId")
            if action_id in actions:
                raise ValueError(f"duplicate encounter actionId: {action_id}")
            actions[action_id] = (pattern.get("patternId"), stage.get("stageId"))

    occurrences: dict[tuple[str, str], dict] = {}
    for binding in pattern_bindings.get("bindings", []):
        action_id = binding.get("actionId")
        if binding.get("suppressAnimation") and binding.get("clips"):
            raise ValueError(f"suppressed action has clips: {action_id}")
        for clip in binding.get("clips", []):
            key = (action_id, clip.get("clipOccurrenceId"))
            if key in occurrences:
                raise ValueError(f"duplicate clip occurrence: {key}")
            occurrences[key] = clip

    binding_ids: set[str] = set()
    occurrence_ids: set[str] = set()
    for cue in document["cues"]:
        if not isinstance(cue, dict) or set(cue) != CUE_FIELDS:
            raise ValueError("sound cue fields are invalid")
        for field in CUE_FIELDS - {"startMs"}:
            value = cue[field]
            if not isinstance(value, str) or not value:
                raise ValueError(f"sound cue {field} must be a non-empty string")
        for field in ("bindingId", "occurrenceId", "patternId", "stageId", "actionId", "clipOccurrenceId"):
            if not STABLE_ID.fullmatch(cue[field]):
                raise ValueError(f"sound cue {field} is not a stable ID")
        if cue["bindingId"] in binding_ids or cue["occurrenceId"] in occurrence_ids:
            raise ValueError("duplicate sound cue identity")
        binding_ids.add(cue["bindingId"])
        occurrence_ids.add(cue["occurrenceId"])
        if cue["repeatPolicy"] not in {"once", "each_loop"}:
            raise ValueError("unsupported sound repeat policy")

        action_id = cue["actionId"]
        expected_stage = actions.get(action_id)
        if expected_stage != (cue["patternId"], cue["stageId"]):
            raise ValueError("sound cue encounter tuple does not resolve exactly")
        clip = occurrences.get((action_id, cue["clipOccurrenceId"]))
        if clip is None:
            raise ValueError("sound cue clip occurrence does not resolve exactly")
        start_ms = cue["startMs"]
        if not isinstance(start_ms, int) or isinstance(start_ms, bool) or start_ms < 0:
            raise ValueError("sound cue startMs is invalid")
        source_start = clip.get("sourceStartMs", 0)
        play_ms = clip.get("playMs", 0)
        if start_ms < source_start or (play_ms and start_ms >= source_start + play_ms):
            raise ValueError("sound cue lies outside its clip occurrence source window")
        if cue["repeatPolicy"] == "each_loop" and not clip.get("loop", False):
            raise ValueError("each_loop sound cue targets a non-loop clip")


class ValtanPatternSoundCueContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.pattern_bindings = json.loads(
            builder.PATTERNBINDINGS_PATH.read_text(encoding="utf-8")
        )
        cls.encounter = json.loads(builder.ENCOUNTER_PATH.read_text(encoding="utf-8"))
        cls.document = json.loads(builder.OUT_PATH.read_text(encoding="utf-8"))

    def test_authored_document_has_exact_join_and_identity_contract(self) -> None:
        validate_document(self.document, self.pattern_bindings, self.encounter)

    def test_authored_document_is_the_deterministic_extracted_projection(self) -> None:
        rows = builder.parse_sound_rows(builder.ANIMEVENTS_PATH)
        cues, stats = builder.build_cues(
            rows,
            builder.build_clip_index(self.pattern_bindings),
            builder.build_action_index(self.encounter),
        )
        cues.sort(
            key=lambda cue: (
                cue["actionId"],
                cue["clipOccurrenceId"],
                cue["startMs"],
                cue["occurrenceId"],
            )
        )
        self.assertEqual(cues, self.document["cues"])
        self.assertEqual(len(cues), stats["matched"])
        self.assertGreater(stats["matched"], 0)

    def test_invalid_identity_join_window_and_repeat_are_fail_closed(self) -> None:
        mutations = []
        duplicate = copy.deepcopy(self.document)
        duplicate["cues"].append(copy.deepcopy(duplicate["cues"][0]))
        mutations.append(duplicate)
        for field, value in (
            ("actionId", "valtan.unknown-action"),
            ("clipOccurrenceId", "valtan.unknown-occurrence"),
            ("stageId", "UNKNOWN_STAGE"),
            ("repeatPolicy", "unknown"),
            ("startMs", -1),
        ):
            malformed = copy.deepcopy(self.document)
            malformed["cues"][0][field] = value
            mutations.append(malformed)
        malformed_version = copy.deepcopy(self.document)
        malformed_version["formatVersion"] = 99
        mutations.append(malformed_version)

        for malformed in mutations:
            with self.subTest(malformed=malformed["cues"][0]):
                with self.assertRaises(ValueError):
                    validate_document(malformed, self.pattern_bindings, self.encounter)

        non_loop_repeat = copy.deepcopy(self.document)
        target = non_loop_repeat["cues"][0]
        target["repeatPolicy"] = "each_loop"
        non_loop_bindings = copy.deepcopy(self.pattern_bindings)
        for binding in non_loop_bindings["bindings"]:
            if binding.get("actionId") != target["actionId"]:
                continue
            for clip in binding.get("clips", []):
                if clip.get("clipOccurrenceId") == target["clipOccurrenceId"]:
                    clip["loop"] = False
        with self.assertRaises(ValueError):
            validate_document(non_loop_repeat, non_loop_bindings, self.encounter)

    def test_product_loader_retains_staged_commit_and_explicit_isolation(self) -> None:
        source = (
            builder.REPO_ROOT / "Client" / "Private" / "ValtanPatternSoundCueDocument.cpp"
        ).read_text(encoding="utf-8-sig")
        staged_at = source.index("VALTAN_PATTERN_SOUND_CUE_DOCUMENT Staged;")
        loop_at = source.index("for (const DATA_JSON_VALUE& CueValue", staged_at)
        sort_at = source.index("std::sort(Staged.Cues.begin()", loop_at)
        commit_at = source.index("InOutDocument = std::move(Staged);", sort_at)
        self.assertLess(staged_at, loop_at)
        self.assertLess(loop_at, sort_at)
        self.assertLess(sort_at, commit_at)
        for token in (
            "pAnimationBinding->bSuppressAnimation && pAnimationBinding->Clips.empty()",
            "iSkippedSuppressedAnimationCount",
            "iSkippedUnimplementedPatternCount",
            "each_loop cue references a non-loop clip",
            "source window is outside its clip segment",
            "Duplicate Valtan action/clip/Sound cue occurrence tuple",
        ):
            self.assertIn(token, source)


if __name__ == "__main__":
    unittest.main()
