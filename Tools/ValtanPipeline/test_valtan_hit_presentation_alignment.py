#!/usr/bin/env python3
"""Focused regression tests for Valtan hit/presentation alignment."""

from __future__ import annotations

import copy
import unittest

from Tools.ValtanPipeline import validate_valtan_hit_presentation_alignment as validator


class ValtanHitPresentationAlignmentTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        root = validator.REPOSITORY_ROOT
        cls.roles = validator._load(root / validator.ROLE_LEDGER_PATH)
        cls.allowlist = validator._load(root / validator.ALLOWLIST_PATH)
        cls.gameplay = validator._load(root / validator.GAMEPLAY_PATH)
        cls.presentation = validator._load(root / validator.PRESENTATION_PATH)
        cls.bindings = validator._load(root / validator.V2_BINDINGS_PATH)
        cls.sounds = validator._load(root / validator.SOUND_CUES_PATH)
        cls.combat_objects = validator._load(root / validator.COMBAT_OBJECTS_PATH)
        cls.combat_sounds = validator._load(
            root / validator.COMBAT_OBJECT_SOUND_CUES_PATH
        )
        cls.boss_catalog = validator._load(root / validator.BOSS_CATALOG_PATH)
        cls.clip_templates = validator._load(root / validator.CLIP_TEMPLATES_PATH)

    def validate(
            self, *, roles=None, allowlist=None, gameplay=None, presentation=None,
            bindings=None, sounds=None, combat_objects=None, combat_sounds=None,
            boss_catalog=None, clip_templates=None,
    ) -> dict[str, int]:
        return validator.validate_alignment(
            roles if roles is not None else self.roles,
            allowlist if allowlist is not None else self.allowlist,
            gameplay if gameplay is not None else self.gameplay,
            presentation if presentation is not None else self.presentation,
            bindings if bindings is not None else self.bindings,
            sounds if sounds is not None else self.sounds,
            combat_objects if combat_objects is not None else self.combat_objects,
            combat_sounds if combat_sounds is not None else self.combat_sounds,
            boss_catalog if boss_catalog is not None else self.boss_catalog,
            clip_templates if clip_templates is not None else self.clip_templates,
        )

    def test_repository_contract_is_complete(self) -> None:
        stats = self.validate()
        self.assertGreater(stats["roleResources"], 0)
        self.assertEqual(stats["attackBindings"], stats["alignedAttackBindings"])
        self.assertGreater(stats["stageHitPoints"], stats["soundAlignedPoints"])
        self.assertGreater(stats["soundTrackExceptions"], 0)
        self.assertGreater(stats["externalBindings"], 0)
        self.assertEqual(stats["combatObjectHits"], stats["combatObjectSoundCues"])
        self.assertGreater(stats["combatV2Contracts"], 0)

    def test_stagger_slot_wipe_effect_is_bound_to_damage_clock(self) -> None:
        binding_ids = {
            "binding.valtan.project-tuned.stagger-slot.final-attack.wipe":
                "boss.valtan.six.sonic",
            "binding.valtan.project-tuned.stagger-slot.final-attack.twohand":
                "boss.valtan.twohand",
        }
        bindings = {
            row["bindingId"]: row
            for row in self.bindings["bindings"]
            if row["bindingId"] in binding_ids
        }
        self.assertEqual(binding_ids.keys(), bindings.keys())
        for binding_id, resource_id in binding_ids.items():
            binding = bindings[binding_id]
            self.assertEqual(resource_id, binding["resource"]["id"])
            self.assertEqual("VALTAN_STAGGER_SLOT", binding["scope"]["patternId"])
            self.assertEqual("FINAL_ATTACK", binding["scope"]["stageId"])
            self.assertEqual("CLIP_OCCURRENCE", binding["clock"]["basis"])
            self.assertEqual(1000, binding["clock"]["startMs"])

        malformed = copy.deepcopy(self.bindings)
        target = next(
            row for row in malformed["bindings"]
            if row["bindingId"] ==
            "binding.valtan.project-tuned.stagger-slot.final-attack.twohand"
        )
        target["clock"]["startMs"] = 900
        with self.assertRaisesRegex(
                validator.ContractError, "attack binding has no hit"):
            self.validate(bindings=malformed)

    def test_source_clock_uses_play_rate(self) -> None:
        occurrence = {
            "wallStartMs": 200.0,
            "sourceStartMs": 200,
            "sourceEndMs": 1200,
            "playRate": 2.0,
        }
        self.assertEqual(
            validator._event_wall_ms(occurrence, 600, "test event"),
            400.0,
        )

    def test_role_coverage_and_attack_binding_timing_fail_closed(self) -> None:
        roles = copy.deepcopy(self.roles)
        roles["resources"] = roles["resources"][1:]
        with self.assertRaisesRegex(validator.ContractError, "effect role coverage drift"):
            self.validate(roles=roles)

        bindings = copy.deepcopy(self.bindings)
        binding = next(
            row for row in bindings["bindings"]
            if row["bindingId"] == "binding.valtan.migrated.006.2e2690c8712917d8"
        )
        binding["clock"]["startMs"] += 100
        with self.assertRaisesRegex(validator.ContractError, "attack binding has no hit"):
            self.validate(bindings=bindings)

    def test_discrete_hit_sound_and_combat_object_sound_fail_closed(self) -> None:
        sounds = copy.deepcopy(self.sounds)
        sounds["cues"] = [
            row for row in sounds["cues"]
            if not (
                row["patternId"] == "VALTAN_SIX_PIZZA_106" and
                row["stageId"] == "STEP_07" and
                "_Shot" in row["soundEvent"]
            )
        ]
        with self.assertRaisesRegex(validator.ContractError, "stage hit lacks impact sound"):
            self.validate(sounds=sounds)

        combat_sounds = copy.deepcopy(self.combat_sounds)
        combat_sounds["cues"] = [
            row for row in combat_sounds["cues"]
            if row["hitId"] != "hit.valtan.high-jump.target-axe.01"
        ]
        with self.assertRaisesRegex(validator.ContractError, "combat-object hit/sound key drift"):
            self.validate(combat_sounds=combat_sounds)

    def test_external_and_sound_track_exceptions_reject_drift(self) -> None:
        allowlist = copy.deepcopy(self.allowlist)
        allowlist["exceptions"] = [
            row for row in allowlist["exceptions"]
            if row["bindingId"] != "binding.valtan.migrated.006.c528dcfd46892776"
        ]
        with self.assertRaisesRegex(
                validator.ContractError, "outside split authoring without exact exception"):
            self.validate(allowlist=allowlist)

        allowlist = copy.deepcopy(self.allowlist)
        track = next(
            row for row in allowlist["exceptions"]
            if row["rule"] == "STAGE_HIT_SOUND_TRACK"
        )
        track["expectedHitOffsetsMs"][-1] += 1
        with self.assertRaisesRegex(validator.ContractError, "exception offsets are stale"):
            self.validate(allowlist=allowlist)


    @staticmethod
    def stage(document, pattern_id, stage_id):
        pattern = next(row for row in document["patterns"] if row["patternId"] == pattern_id)
        return next(row for row in pattern["stages"] if row["stageId"] == stage_id)

    def test_reviewed_receipts_preserve_shared_attack_roles(self) -> None:
        stats = self.validate()
        self.assertEqual(1, stats["presentationOnlyBindings"])
        self.assertEqual(5, stats["authoredSoundExceptions"])
        roles = {row["id"]: row for row in self.roles["resources"]}
        self.assertEqual("ATTACK", roles["boss.valtan.shout.burst"]["role"])
        self.assertEqual("CLIP_TEMPLATE", roles["boss.valtan.shout.burst"]["alignmentPolicy"])
        self.assertEqual("ATTACK", roles["boss.valtan.six.sonic"]["role"])
        self.assertEqual("STATE", roles["boss.valtan.six.sonic.after"]["role"])

    def test_presentation_only_receipt_requires_exact_binding(self) -> None:
        target_id = "binding.valtan.authored.ff796eec4443072d"
        for mutation in ("clock", "resource", "scope", "identity"):
            with self.subTest(mutation=mutation):
                bindings = copy.deepcopy(self.bindings)
                row = next(row for row in bindings["bindings"] if row["bindingId"] == target_id)
                if mutation == "clock":
                    row["clock"]["startMs"] += 1
                elif mutation == "resource":
                    row["resource"]["id"] = "boss.valtan.impact"
                elif mutation == "scope":
                    row["scope"]["stageId"] = "SILENCE_APPLY"
                    row["scope"]["actionId"] = "valtan.authoring.silence-slot.apply"
                else:
                    row["bindingId"] += ".changed"
                with self.assertRaises(validator.ContractError):
                    self.validate(bindings=bindings)

        allowlist = copy.deepcopy(self.allowlist)
        allowlist["exceptions"] = [row for row in allowlist["exceptions"]
                                   if row["rule"] != "PROJECT_AUTHORED_PRESENTATION_ONLY"]
        with self.assertRaisesRegex(validator.ContractError, "CLIP_TEMPLATE binding lacks"):
            self.validate(allowlist=allowlist)

    def test_presentation_only_receipt_rejects_damage_or_source_clips(self) -> None:
        for mutation in ("hit", "damage_event", "mapping", "duration"):
            with self.subTest(mutation=mutation):
                gameplay = copy.deepcopy(self.gameplay)
                presentation = copy.deepcopy(self.presentation)
                stage = self.stage(gameplay, "VALTAN_SILENCE_SLOT", "STEP_01")
                if mutation == "hit":
                    stage["hit"] = copy.deepcopy(self.stage(gameplay, "VALTAN_FLOOR_WIPE_130", "FIRST_SMASH")["hit"])
                elif mutation == "damage_event":
                    stage["events"].append({"kind": "DAMAGE_GRABBED_PLAYERS"})
                elif mutation == "duration":
                    stage["durationMs"] = 786
                else:
                    pstage = self.stage(presentation, "VALTAN_SILENCE_SLOT", "STEP_01")
                    pstage["animation"]["occurrences"][0]["mappingBasis"] = "SOURCE_RESTORED"
                with self.assertRaises(validator.ContractError):
                    self.validate(gameplay=gameplay, presentation=presentation)

    def test_new_receipt_fields_and_stale_presentation_fail_closed(self) -> None:
        for rule, field in (("PROJECT_AUTHORED_PRESENTATION_ONLY", "resource"),
                            ("PROJECT_AUTHORED_PRESENTATION_ONLY", "clock"),
                            ("PROJECT_AUTHORED_PRESENTATION_ONLY", "mappingBasis"),
                            ("PROJECT_AUTHORED_SOUND_TIMING", "expectedSoundCues"),
                            ("PROJECT_AUTHORED_SOUND_TIMING", "expectedAnimation")):
            with self.subTest(rule=rule, field=field):
                allowlist = copy.deepcopy(self.allowlist)
                row = next(row for row in allowlist["exceptions"] if row["rule"] == rule)
                del row[field]
                with self.assertRaisesRegex(validator.ContractError, "fields must be"):
                    self.validate(allowlist=allowlist)
        bindings = copy.deepcopy(self.bindings)
        bindings["bindings"] = [row for row in bindings["bindings"]
                                if row["bindingId"] != "binding.valtan.authored.ff796eec4443072d"]
        with self.assertRaisesRegex(validator.ContractError, "stale hit-alignment"):
            self.validate(bindings=bindings)

    def test_authored_sound_receipts_require_full_saved_payload(self) -> None:
        for mutation in ("time", "event", "window", "remove", "add"):
            with self.subTest(mutation=mutation):
                sounds = copy.deepcopy(self.sounds)
                cue = next(row for row in sounds["cues"]
                           if row["patternId"] == "VALTAN_WHIRLWIND" and row["stageId"] == "SPIN")
                if mutation == "time":
                    cue["startMs"] += 1
                elif mutation == "event":
                    cue["soundEvent"] = "G_Voltan2_Attack15_Shot1"
                elif mutation == "window":
                    cue["playbackDurationMs"] = 123
                elif mutation == "remove":
                    sounds["cues"].remove(cue)
                else:
                    extra = copy.deepcopy(cue)
                    extra["bindingId"] += ".extra"
                    extra["occurrenceId"] += ".extra"
                    sounds["cues"].append(extra)
                with self.assertRaisesRegex(validator.ContractError, "authored sound receipt drift"):
                    self.validate(sounds=sounds)

    def test_authored_sound_receipts_reject_hit_drift_and_absence(self) -> None:
        gameplay = copy.deepcopy(self.gameplay)
        stage = self.stage(gameplay, "VALTAN_WHIRLWIND", "SPIN")
        stage["hit"]["schedule"]["intervalMs"] += 1
        with self.assertRaisesRegex(validator.ContractError, "authored sound exception hit offsets are stale"):
            self.validate(gameplay=gameplay)
        allowlist = copy.deepcopy(self.allowlist)
        allowlist["exceptions"] = [row for row in allowlist["exceptions"]
                                   if row["rule"] != "PROJECT_AUTHORED_SOUND_TIMING"]
        with self.assertRaisesRegex(validator.ContractError, "stage hit lacks impact sound"):
            self.validate(allowlist=allowlist)

    def test_authored_sound_receipts_pin_animation_wall_clock(self) -> None:
        for mutation in ("rate", "source_start", "prefix", "extra_occurrence"):
            with self.subTest(mutation=mutation):
                presentation = copy.deepcopy(self.presentation)
                animation = self.stage(presentation, "VALTAN_FOUR_SLASH", "SLASHES")["animation"]
                occurrences = animation["occurrences"]
                if mutation == "rate":
                    occurrences[0]["playRate"] *= 2.0
                elif mutation == "source_start":
                    occurrences[0]["sourceStartMs"] += 1
                else:
                    extra = copy.deepcopy(occurrences[0])
                    extra["clipOccurrenceId"] += ".clock-receipt-test"
                    extra["sourceStartMs"] = 0
                    extra["playMs"] = 100
                    if mutation == "prefix":
                        occurrences.insert(0, extra)
                    else:
                        occurrences.append(extra)
                with self.assertRaisesRegex(validator.ContractError, "authored sound animation receipt drift"):
                    self.validate(presentation=presentation)

    def test_sound_receipt_compares_json_types_exactly(self) -> None:
        allowlist = copy.deepcopy(self.allowlist)
        row = next(row for row in allowlist["exceptions"]
                   if row["rule"] == "PROJECT_AUTHORED_SOUND_TIMING")
        cue = row["expectedSoundCues"][0]
        cue["startMs"] = float(cue["startMs"])
        with self.assertRaisesRegex(validator.ContractError, "authored sound receipt drift"):
            self.validate(allowlist=allowlist)

    def test_existing_high_frequency_exception_does_not_cover_authored_timing(self) -> None:
        allowlist = copy.deepcopy(self.allowlist)
        row = next(row for row in allowlist["exceptions"]
                   if row["rule"] == "PROJECT_AUTHORED_SOUND_TIMING")
        row["rule"] = "STAGE_HIT_SOUND_TRACK"
        del row["expectedSoundCues"]
        del row["expectedAnimation"]
        with self.assertRaisesRegex(validator.ContractError, "only valid for <=100ms high-frequency"):
            self.validate(allowlist=allowlist)


if __name__ == "__main__":
    unittest.main()
