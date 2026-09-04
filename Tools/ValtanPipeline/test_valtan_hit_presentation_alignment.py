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
        binding = next(
            row for row in self.bindings["bindings"]
            if row["bindingId"] ==
            "binding.valtan.project-tuned.stagger-slot.final-attack.wipe"
        )
        self.assertEqual("boss.valtan.six.sonic", binding["resource"]["id"])
        self.assertEqual("VALTAN_STAGGER_SLOT", binding["scope"]["patternId"])
        self.assertEqual("FINAL_ATTACK", binding["scope"]["stageId"])
        self.assertEqual("CLIP_OCCURRENCE", binding["clock"]["basis"])
        self.assertEqual(2900, binding["clock"]["startMs"])

        malformed = copy.deepcopy(self.bindings)
        target = next(
            row for row in malformed["bindings"]
            if row["bindingId"] == binding["bindingId"]
        )
        target["clock"]["startMs"] = 2800
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
            if row["bindingId"] == "binding.valtan.project-tuned.six-pizza.stomp"
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


if __name__ == "__main__":
    unittest.main()
