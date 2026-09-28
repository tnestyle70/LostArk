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
        cls.full_restore_animations = validator._load(root / validator.FULL_RESTORE_ANIMATIONS_PATH)

    def validate(self, *, templates=None, gameplay=None, presentation=None,
                 bindings=None, sounds=None, full_restore_animations=None) -> dict[str, int]:
        return validator.validate_parity(
            templates if templates is not None else self.templates,
            gameplay if gameplay is not None else self.gameplay,
            presentation if presentation is not None else self.presentation,
            bindings if bindings is not None else self.bindings,
            sounds if sounds is not None else self.sounds,
            full_restore_animations if full_restore_animations is not None else self.full_restore_animations,
        )

    def test_repository_contract_is_complete(self) -> None:
        stats = self.validate()
        self.assertGreaterEqual(stats["templates"], 12)
        self.assertGreater(stats["occurrences"], stats["templates"])
        self.assertGreater(stats["hits"], 0)
        self.assertGreater(stats["effects"], 0)
        self.assertGreater(stats["sounds"], 0)
        self.assertEqual(2, stats["effectReplacements"])
        self.assertEqual(2, stats["effectTimingOverrides"])

    def test_exact_saved_effect_timing_keeps_independent_stage_cue(self) -> None:
        override_row = next(row for row in self.templates["allowlist"]
                            if "effectTimingOverride" in row)
        self.assertEqual(("VALTAN_TRIPLE_COUNTER", "FAIL_3"),
                         (override_row["patternId"], override_row["stageId"]))
        self.assertEqual(1169, override_row["effectTimingOverride"]["stageStartMs"])
        self.assertEqual(900, override_row["effectTimingOverride"]["templateEffect"]["clipMs"])
        presentation = copy.deepcopy(self.presentation)
        stage = next(stage for pattern in presentation["patterns"]
                     if pattern["patternId"] == override_row["patternId"]
                     for stage in pattern["stages"] if stage["stageId"] == override_row["stageId"])
        cue = next(cue for cue in stage["effectCues"]
                   if cue["cueId"] == "cue.valtan.composition.valtan_triple_counter.fail_3.01")
        self.assertEqual(("STAGE_CLOCK", 891), (cue["timingBasis"], cue["stageOffsetMs"]))
        stats = self.validate()
        stage["effectCues"].remove(cue)
        self.assertEqual(stats, self.validate(presentation=presentation))

    def test_effect_timing_override_binding_identity_and_fields_fail_closed(self) -> None:
        override = next(row["effectTimingOverride"] for row in self.templates["allowlist"]
                        if "effectTimingOverride" in row)
        for section, field, value in (
            (None, "bindingId", "binding.fixture.wrong-id"),
            ("scope", "patternId", "VALTAN_THREE"),
            ("scope", "stageId", "FAIL_2"),
            ("scope", "actionId", "valtan.fixture.wrong-action"),
            ("resource", "id", "boss.valtan.shout"),
            ("clock", "startMs", 1168),
            ("clock", "startMs", 900),
            ("clock", "basis", "CLIP_OCCURRENCE"),
            ("clock", "clipOccurrenceId", "valtan.fixture.wrong-clip"),
            ("clock", "repeatPolicy", "LOOP"),
            ("anchor", "slotId", "root"),
        ):
            bindings = copy.deepcopy(self.bindings)
            binding = next(row for row in bindings["bindings"]
                           if row["bindingId"] == override["bindingId"])
            (binding if section is None else binding[section])[field] = value
            with self.subTest(section=section, field=field, value=value), self.assertRaisesRegex(
                    validator.ContractError, "exact effect timing override binding mismatch"):
                self.validate(bindings=bindings)

    def test_effect_timing_override_missing_and_duplicate_binding_fail_closed(self) -> None:
        override = next(row["effectTimingOverride"] for row in self.templates["allowlist"]
                        if "effectTimingOverride" in row)
        for duplicate_kind in ("missing", "same-id", "other-id", "original-stage", "original-clip"):
            bindings = copy.deepcopy(self.bindings)
            binding = next(row for row in bindings["bindings"]
                           if row["bindingId"] == override["bindingId"])
            if duplicate_kind == "missing":
                bindings["bindings"].remove(binding)
            else:
                duplicate = copy.deepcopy(binding)
                if duplicate_kind != "same-id":
                    duplicate["bindingId"] = "binding.fixture.duplicate"
                if duplicate_kind.startswith("original-"):
                    duplicate["clock"]["startMs"] = override["templateEffect"]["clipMs"]
                if duplicate_kind == "original-clip":
                    duplicate["clock"]["basis"] = "CLIP_OCCURRENCE"
                    duplicate["clock"]["clipOccurrenceId"] = \
                        "valtan.reactive.triple-counter.third-fail.clip.01"
                bindings["bindings"].append(duplicate)
            with self.subTest(kind=duplicate_kind), self.assertRaisesRegex(
                    validator.ContractError, "exact effect timing override binding mismatch"):
                self.validate(bindings=bindings)

    def test_effect_timing_override_is_exact_and_cannot_waive_validation(self) -> None:
        for change in ("unknown-field", "waiver", "wrong-template", "unknown-binding",
                       "unchanged-time", "outside-stage", "missing-override"):
            templates = copy.deepcopy(self.templates)
            row = next(row for row in templates["allowlist"] if "effectTimingOverride" in row)
            override = row["effectTimingOverride"]
            if change == "unknown-field":
                override["unknown"] = 1
            elif change == "waiver":
                row["waivers"] = ["EFFECT"]
            elif change == "wrong-template":
                override["templateEffect"]["resourceId"] = "boss.valtan.shout"
            elif change == "unknown-binding":
                override["bindingId"] = "binding.fixture.unknown"
            elif change == "unchanged-time":
                override["stageStartMs"] = 900
            elif change == "outside-stage":
                override["stageStartMs"] = 999999
            else:
                templates["allowlist"].remove(row)
            with self.subTest(change=change), self.assertRaises(validator.ContractError):
                self.validate(templates=templates)

    def test_full_restore_replacements_keep_exact_source_cue_and_no_generic_duplicate(self) -> None:
        for stage_id in ("STEP_03", "STEP_04"):
            for field, value in (("effectAssetId", "effect.missing"),
                                 ("sourceStartMs", 1), ("anchorSlotId", "b_root")):
                presentation = copy.deepcopy(self.presentation)
                stage = next(s for p in presentation["patterns"] if p["patternId"] == "VALTAN_SIX_PIZZA_106"
                             for s in p["stages"] if s["stageId"] == stage_id)
                stage["effectCues"][0][field] = value
                with self.subTest(stage=stage_id, field=field), self.assertRaises(validator.ContractError):
                    self.validate(presentation=presentation)
            metadata = copy.deepcopy(self.full_restore_animations)
            replacement = next(r["effectReplacement"] for r in self.templates["allowlist"]
                               if r["patternId"] == "VALTAN_SIX_PIZZA_106" and r["stageId"] == stage_id)
            next(r for r in metadata["effects"] if r["effectAssetId"] == replacement["effectAssetId"])["animationClips"][0]["clipName"] = "wrong_clip"
            with self.assertRaises(validator.ContractError):
                self.validate(full_restore_animations=metadata)
            bindings = copy.deepcopy(self.bindings)
            expected = replacement["templateEffect"]
            bindings["bindings"].append({
                "resource": {"kind": expected["resourceKind"], "id": expected["resourceId"]},
                "scope": {"patternId": "VALTAN_SIX_PIZZA_106", "stageId": stage_id,
                          "actionId": "valtan.sequence.center-six-pizza-charge." + stage_id.lower().replace("_", "-")},
            })
            with self.assertRaises(validator.ContractError):
                self.validate(bindings=bindings)

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
