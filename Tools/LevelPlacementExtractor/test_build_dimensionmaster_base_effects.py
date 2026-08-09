#!/usr/bin/env python3

from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path

from build_dimensionmaster_base_effects import (
    PARTIAL_RUNTIME_MODULE_CLASSES,
    dimensionmaster_admitted_skills,
)


class DimensionMasterBaseEffectTests(unittest.TestCase):
    def test_admission_uses_current_effect_id_and_skillbinding(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            player_skills = root / "PlayerSkills.json"
            bindings = root / "DimensionMaster.skillbindings.json"
            player_skills.write_text(json.dumps({"skills": [
                {
                    "skillId": 10,
                    "characterClass": "DIMENSIONMASTER",
                    "inputSlot": "LMB",
                    "skillKind": "COMBO",
                    "effectId": "effect.dimensionmaster.skill.10",
                    "comboStages": [{}, {}],
                },
                {
                    "skillId": 20,
                    "characterClass": "DIMENSIONMASTER",
                    "inputSlot": "Q",
                    "skillKind": "ACTIVE",
                    "effectId": "",
                    "comboStages": [],
                },
            ]}), encoding="utf-8")
            bindings.write_text(json.dumps({
                "characterClass": "DIMENSIONMASTER",
                "bindings": [
                    {
                        "skillId": 10,
                        "clips": [
                            {"clip": "ba1", "playMs": 100, "playRate": 1.0},
                            {"clip": "ba2", "playMs": 200, "playRate": 0.5},
                        ],
                    },
                    {"skillId": 20, "clips": ["candidate_clip"]},
                ],
            }), encoding="utf-8")

            admitted = dimensionmaster_admitted_skills(
                player_skills, bindings
            )

            self.assertEqual([10], [row["skillId"] for row in admitted])
            self.assertEqual("BA", admitted[0]["inputSlot"])
            self.assertEqual(["ba1", "ba2"], admitted[0]["clips"])
            self.assertEqual(100, admitted[0]["clipBindings"][0]["playMs"])

    def test_partial_module_contract_never_claims_required_as_exact(self) -> None:
        self.assertIn("particlemodulelifetime", PARTIAL_RUNTIME_MODULE_CLASSES)
        self.assertNotIn("particlemodulerequired", PARTIAL_RUNTIME_MODULE_CLASSES)

    def test_combo_stage_clip_groups_are_flattened_for_admission(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            player_skills = root / "PlayerSkills.json"
            bindings = root / "DimensionMaster.skillbindings.json"
            player_skills.write_text(json.dumps({"skills": [{
                "skillId": 10,
                "characterClass": "DIMENSIONMASTER",
                "inputSlot": "LMB",
                "skillKind": "COMBO",
                "effectId": "effect.dimensionmaster.skill.10",
                "comboStages": [{}, {}],
            }]}), encoding="utf-8")
            bindings.write_text(json.dumps({
                "characterClass": "DIMENSIONMASTER",
                "bindings": [{
                    "skillId": 10,
                    "clips": [["ba1"], ["ba2"]],
                }],
            }), encoding="utf-8")

            admitted = dimensionmaster_admitted_skills(player_skills, bindings)

            self.assertEqual(["ba1", "ba2"], admitted[0]["clips"])
            self.assertEqual([["ba1"], ["ba2"]], admitted[0]["clipBindings"])


if __name__ == "__main__":
    unittest.main()
