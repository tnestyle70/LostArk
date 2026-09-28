from __future__ import annotations

import copy
import os
import subprocess
import unittest
from pathlib import Path

from Tools.ValtanPipeline import valtan_tuning_pipeline as pipeline
from Tools.ValtanPipeline.test_valtan_combat_object_typed_writer import definition, visual


ROOT = Path(__file__).resolve().parents[2]


def fixture(count: int) -> tuple[dict, dict]:
    objects, visuals = [], []
    for ordinal in range(count):
        obj, row = definition(), visual()
        suffix = f".capacity-{ordinal}"
        obj["combatObjectArchetypeId"] += suffix
        obj["hits"][0]["hitId"] += suffix
        row["combatObjectArchetypeId"] += suffix
        row["clientVisualId"] += suffix
        objects.append(obj)
        visuals.append(row)
    return (
        {"bosses": [{"archetypeId": "BOSS_VALTAN", "combatObjectVisuals": visuals}]},
        {"schema": "lostark.valtan-combat-object-authoring", "formatVersion": 1,
         "encounterId": "ENCOUNTER_VALTAN", "objects": objects},
    )


class ValtanCombatVisualCapacityTests(unittest.TestCase):
    def test_canonical_projection_and_visual_closure_share_bounds(self) -> None:
        for count in (1, 18, 32):
            with self.subTest(count=count):
                catalog, combat = fixture(count)
                self.assertEqual(count, len(pipeline._boss_visuals(catalog)))
                pipeline.validate_combat_object_visual_closure(catalog, combat)
        for count in (0, 33):
            with self.subTest(count=count):
                catalog, combat = fixture(count)
                with self.assertRaisesRegex(pipeline.PipelineError, "1..32"):
                    pipeline._boss_visuals(catalog)
                with self.assertRaisesRegex(pipeline.PipelineError, "1..32"):
                    pipeline.validate_combat_object_visual_closure(catalog, combat)

    def test_maximum_capacity_preserves_group_validation(self) -> None:
        catalog, combat = fixture(32)
        row = catalog["bosses"][0]["combatObjectVisuals"][0]
        row["effectV2Group"] = {
            "groupId": "boss.valtan.capacity.group", "playbackRate": 1.0,
            "visualHitMs": 500, "serverHitId": combat["objects"][0]["hits"][0]["hitId"],
        }
        pipeline.validate_combat_object_visual_closure(catalog, combat)
        for changes in ({"unknownField": True}, {"playbackRate": 0}, {"visualHitMs": 501}):
            with self.subTest(changes=changes):
                bad = copy.deepcopy(catalog)
                bad["bosses"][0]["combatObjectVisuals"][0]["effectV2Group"].update(changes)
                with self.assertRaises(pipeline.PipelineError):
                    pipeline.validate_combat_object_visual_closure(bad, combat)

    @unittest.skipUnless(os.name == "nt", "Production publisher uses Windows PowerShell")
    def test_actual_publisher_owner_guard_accepts_32_rejects_33(self) -> None:
        source = (ROOT / "Tools/GameplayPipeline/Publish-GameplayBalance.ps1").read_text(encoding="utf-8-sig")
        start = source.index("$bossCatalogOwners = @(")
        guard = source[start:source.index("$presentationPartById = @{}", start)]
        script = "$ErrorActionPreference='Stop'\n" + "\n".join(
            "$bossPartDocument = [pscustomobject]@{ bossArchetypeId='BOSS_VALTAN'; parts=@() }\n"
            f"$bossCatalogDocument = [pscustomobject]@{{ bosses=@([pscustomobject]@{{ archetypeId='BOSS_VALTAN'; armorParts=@(); combatObjectVisuals=@(1) * {count} }}) }}\n"
            "$accepted=$true\ntry {\n" + guard + "\n} catch { $accepted=$false }\n"
            f"if ($accepted -ne ${str(expected).lower()}) {{ throw 'Unexpected capacity result: {count}' }}"
            for count, expected in ((0, False), (1, True), (18, True), (32, True), (33, False))
        )
        result = subprocess.run(["powershell", "-NoProfile", "-Command", script], capture_output=True, text=True)
        self.assertEqual(0, result.returncode, result.stdout + result.stderr)


if __name__ == "__main__":
    unittest.main()
