#!/usr/bin/env python3
"""Focused regression tests for Valtan clip-template parity."""

from __future__ import annotations

import copy
import unittest

from Tools.ValtanPipeline import validate_valtan_clip_template_parity as validator


class ValtanClipTemplateParityTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        root = validator.REPOSITORY_ROOT
        cls.templates = validator._load(root / validator.TEMPLATE_PATH)
        cls.gameplay = validator._load(root / validator.GAMEPLAY_PATH)
        cls.presentation = validator._load(root / validator.PRESENTATION_PATH)
        cls.bindings = validator._load(root / validator.V2_BINDINGS_PATH)
        cls.sounds = validator._load(root / validator.SOUND_CUES_PATH)

    def validate(self, *, templates=None, gameplay=None, presentation=None,
                 bindings=None, sounds=None) -> dict[str, int]:
        return validator.validate_parity(
            templates if templates is not None else self.templates,
            gameplay if gameplay is not None else self.gameplay,
            presentation if presentation is not None else self.presentation,
            bindings if bindings is not None else self.bindings,
            sounds if sounds is not None else self.sounds,
        )

    def test_repository_contract_is_complete(self) -> None:
        stats = self.validate()
        self.assertGreaterEqual(stats["templates"], 12)
        self.assertGreater(stats["occurrences"], stats["templates"])
        self.assertGreater(stats["hits"], 0)
        self.assertGreater(stats["effects"], 0)
        self.assertGreater(stats["sounds"], 0)

    def test_missing_hit_effect_and_sound_each_fail_closed(self) -> None:
        gameplay = copy.deepcopy(self.gameplay)
        three = next(row for row in gameplay["patterns"]
                     if row["patternId"] == "VALTAN_THREE")
        step_one = next(row for row in three["stages"]
                        if row["stageId"] == "STEP_01")
        step_one["hit"]["schedule"]["offsetsMs"] = [1200]

        bindings = copy.deepcopy(self.bindings)
        bindings["bindings"] = [row for row in bindings["bindings"] if not (
            row.get("scope", {}).get("patternId") == "VALTAN_THREE" and
            row.get("scope", {}).get("stageId") == "STEP_01" and
            row.get("resource", {}).get("id") == "boss.valtan.twohand"
        )]

        sounds = copy.deepcopy(self.sounds)
        sounds["cues"] = [row for row in sounds["cues"] if not (
            row.get("patternId") == "VALTAN_THREE" and
            row.get("stageId") == "STEP_01" and
            row.get("soundEvent") == "G_Voltan2_Attack02_Shot1" and
            row.get("startMs") == 1617
        )]

        for name, values in (
            ("hit", {"gameplay": gameplay}),
            ("effect", {"bindings": bindings}),
            ("sound", {"sounds": sounds}),
        ):
            with self.subTest(name=name), self.assertRaises(validator.ContractError):
                self.validate(**values)

    def test_shape_response_and_unknown_occurrence_fail_closed(self) -> None:
        gameplay = copy.deepcopy(self.gameplay)
        three = next(row for row in gameplay["patterns"]
                     if row["patternId"] == "VALTAN_THREE")
        step_one = next(row for row in three["stages"]
                        if row["stageId"] == "STEP_01")
        step_one["hit"]["shape"]["lengthM"] = 14.0
        with self.assertRaises(validator.ContractError):
            self.validate(gameplay=gameplay)

        templates = copy.deepcopy(self.templates)
        templates["allowlist"][0]["clipOccurrenceId"] = "unknown.occurrence"
        with self.assertRaises(validator.ContractError):
            self.validate(templates=templates)

    def test_stale_allowlist_waiver_fails_closed(self) -> None:
        templates = copy.deepcopy(self.templates)
        templates["allowlist"].append({
            "patternId": "VALTAN_THREE",
            "stageId": "STEP_01",
            "actionId": "valtan.sequence.three.step-01",
            "clipOccurrenceId": "valtan.sequence.three.step-01.clip-01",
            "waivers": ["EFFECT"],
            "reason": "Negative regression fixture deliberately adds a stale waiver.",
        })
        with self.assertRaises(validator.ContractError):
            self.validate(templates=templates)

    def test_reviewed_extra_hit_waiver_is_exact_and_fail_closed(self) -> None:
        templates = copy.deepcopy(self.templates)
        waiver = next(row for row in templates["allowlist"] if (
            row["patternId"] == "VALTAN_THREE" and
            row["stageId"] == "STEP_03"
        ))
        self.assertEqual(["EXTRA_HIT"], waiver["waivers"])
        self.assertEqual([500], waiver["extraHitOffsetsMs"])

        waiver["extraHitOffsetsMs"] = [501]
        with self.assertRaises(validator.ContractError):
            self.validate(templates=templates)

    def test_playback_rate_uses_source_span_and_wall_clock_offset(self) -> None:
        occurrence = {
            "clipOccurrenceId": "fixture.rate-two",
            "sourceStartMs": 100,
            "playMs": 800,
            "playRate": 2.0,
        }
        self.assertEqual(400, validator._occurrence_wall_duration_ms(occurrence))
        self.assertEqual(225, validator._event_stage_ms(occurrence, 200, 150))
        self.assertIsNone(validator._event_stage_ms(occurrence, 200, 900))


if __name__ == "__main__":
    unittest.main()
