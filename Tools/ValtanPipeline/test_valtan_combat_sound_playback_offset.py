"""Combat semantic sound offsets skip WAV lead-in without moving Server hit clocks."""
import copy
import unittest

from Tools.CompositionPipeline import composition_pipeline as composition
from Tools.ValtanPipeline import validate_valtan_hit_presentation_alignment as alignment


class CombatSoundPlaybackOffsetTests(unittest.TestCase):
    def setUp(self):
        self.cue = dict(bindingId="sound.axe", combatObjectArchetypeId="object.axe",
                        hitId="hit.axe", soundBank="S_Mob_G_Voltan2",
                        soundEvent="G_Voltan2_Attack09_ProjExp1")
        self.document = dict(schema="lostark.valtan-combat-object-sound-cues",
                             formatVersion=1, ownerArchetypeId="BOSS_VALTAN", cues=[self.cue])
        self.objects = dict(schema="lostark.valtan-combat-object-authoring", formatVersion=1,
                            encounterId="ENCOUNTER_VALTAN", objects=[dict(combatObjectArchetypeId="object.axe",
                                         hits=[dict(hitId="hit.axe")])])

    def validate(self):
        composition._validate_combat_object_sound_document(
            self.document, {self.cue["soundEvent"]}, self.objects)
        return alignment._validate_combat_objects(self.objects, self.document)

    def test_legacy_default_and_valid_source_offsets_keep_hit_identity(self):
        for value in (None, 0, 50, 600000):
            with self.subTest(offset=value):
                self.setUp()
                if value is not None:
                    self.cue["playbackOffsetMs"] = value
                before = copy.deepcopy((self.objects, self.document))
                hits, count = self.validate()
                self.assertEqual({("object.axe", "hit.axe")}, set(hits))
                self.assertEqual(1, count)
                self.assertEqual(before, (self.objects, self.document))

    def test_both_publish_validators_reject_malformed_source_offsets(self):
        for invalid in (-1, 600001, 0.5, True, None, "50"):
            with self.subTest(offset=invalid):
                self.setUp()
                self.cue["playbackOffsetMs"] = invalid
                before = copy.deepcopy(self.document)
                with self.assertRaises(composition.CompositionError):
                    composition._validate_combat_object_sound_document(
                        self.document, {self.cue["soundEvent"]}, self.objects)
                with self.assertRaises(alignment.ContractError):
                    alignment._validate_combat_objects(self.objects, self.document)
                self.assertEqual(before, self.document)

    def test_unknown_fields_still_fail_closed(self):
        self.cue["offsetMs"] = 50
        with self.assertRaises(composition.CompositionError):
            composition._validate_combat_object_sound_document(
                self.document, {self.cue["soundEvent"]}, self.objects)
        with self.assertRaises(alignment.ContractError):
            alignment._validate_combat_objects(self.objects, self.document)


if __name__ == "__main__":
    unittest.main()
