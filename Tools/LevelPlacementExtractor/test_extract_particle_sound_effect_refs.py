#!/usr/bin/env python3

from __future__ import annotations

import importlib.util
import tempfile
import unittest
from pathlib import Path


MODULE_PATH = Path(__file__).with_name("extract_particle_sound_effect_refs.py")
SPEC = importlib.util.spec_from_file_location(
    "extract_particle_sound_effect_refs", MODULE_PATH
)
MODULE = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(MODULE)


class ParticleSoundEffectReferenceTests(unittest.TestCase):
    def test_typed_references_are_aggregated_without_name_guessing(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            first = root / "9_BaseBuff_Valtan_Rage.loa"
            second = root / "9_BaseBuff_Valtan_Barrier.loa"
            first.write_bytes(
                b"ParticleSystem'FX_TEST.Par_Rage'\x00"
                b"Material'FX_MAT.M_Rage'\x00"
            )
            second.write_bytes(
                b"ParticleSystem'FX_TEST.Par_Rage'\x00"
                b"AkEvent'S_Buff.Rage'\x00"
            )

            document = MODULE.extract_document([first, second], "VALTAN_BASEBUFF")

            self.assertEqual(2, document["summary"]["sourceFileCount"])
            self.assertEqual(1, document["summary"]["uniqueParticleSystemCount"])
            self.assertEqual(2, document["summary"]["particleSystemOccurrenceCount"])
            self.assertEqual(
                ["Barrier", "Rage"],
                document["particleSystems"][0]["actionNames"],
            )
            self.assertEqual(1, document["summary"]["uniqueMaterialCount"])
            self.assertEqual(1, document["summary"]["uniqueAudioEventCount"])


if __name__ == "__main__":
    unittest.main()
