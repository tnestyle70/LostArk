"""Execute the typed Object editor patch through source validation and projection."""
import copy
import json
from pathlib import Path
import unittest

import valtan_tuning_pipeline as pipeline

ROOT = Path(__file__).resolve().parents[2]


class ObjectImpactTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.docs = pipeline.load_pipeline_documents(ROOT)
        cls.revision = pipeline.source_manifest(ROOT)["sourceManifestId"]
        cls.master = pipeline.join_v2_authoring(*(cls.docs[key] for key in (
            pipeline.GAMEPLAY_AUTHORING_REL, pipeline.PRESENTATION_AUTHORING_REL,
            pipeline.WORLD_SET_REL, pipeline.COMBAT_AUTHORING_REL)))

    def patch(self, archetype="combatobject.valtan.struggling.rock-pillar", **fields):
        owner, stage = next((p, s) for p in self.master["patterns"] for s in p["stages"]
                            if any(e.get("combatObjectArchetypeId") == archetype for e in s.get("events", [])))
        obj = next(o for o in self.docs[pipeline.COMBAT_AUTHORING_REL]["objects"]
                   if o["combatObjectArchetypeId"] == archetype)
        hit = obj["hits"][0]
        op = dict(op="SET_COMBAT_OBJECT_IMPACT", patternId=owner["patternId"], stageId=stage["stageId"],
                  combatObjectArchetypeId=archetype, hitId=hit["hitId"],
                  atMs=hit["trigger"]["atMs"], innerRadiusM=0, outerRadiusM=4.25,
                  pushRangeM=2.75, pushMs=230, knockdown=True, downMs=1700,
                  chainDelayMs=obj.get("ownerHitChain", {}).get("delayMs", 0))
        op.update(fields)
        return dict(schema=pipeline.DRAFT_PATCH_SCHEMA, formatVersion=1,
                    sourceRevision=self.revision, operations=[op])

    def apply(self, patch):
        return pipeline.apply_draft_patch(
            self.master, self.docs[pipeline.BOSS_PROFILES_REL], self.docs[pipeline.DAMAGE_REL],
            patch, self.revision, self.docs[pipeline.WORLD_SET_REL], self.docs[pipeline.COMBAT_AUTHORING_REL],
            repository_root=ROOT, effect_catalog=self.docs[pipeline.EFFECT_CATALOG_REL], include_combat_authoring=True)

    def test_fixed_explosion_projects_clock_damage_and_knockback(self):
        original = copy.deepcopy(self.docs[pipeline.COMBAT_AUTHORING_REL])
        master, bosses, damage, combat, count = self.apply(self.patch(atMs=4200))
        self.assertEqual(count, 1)
        self.assertEqual(original, self.docs[pipeline.COMBAT_AUTHORING_REL])
        docs = dict(self.docs); docs[pipeline.COMBAT_AUTHORING_REL] = combat
        product = json.loads(pipeline.project_v2_products(ROOT, docs, master)[pipeline.COMBAT_PRODUCT_REL])
        obj = next(o for o in product["objects"] if o["combatObjectArchetypeId"] == "combatobject.valtan.struggling.rock-pillar")
        hit = obj["hits"][0]
        self.assertEqual((hit["atMs"], hit["hitOuterRadius"], hit["pushRangeM"], hit["pushMs"], hit["downMs"]),
                         (4200, 4.25, 2.75, 230, 1700))
        self.assertEqual(hit["serverDamageProfileId"], "damage.valtan.stomp")
        preparation = next(e for e in obj["presentationEvents"] if e["presentationEventId"].endswith(".prepare"))
        self.assertEqual(preparation["atMs"], 4200 - 1820)

    def test_chain_explosion_retains_owner_event_and_changes_delay(self):
        result = self.apply(self.patch("combatobject.valtan.six-pizza.rock-pillar", chainDelayMs=1900))
        obj = next(o for o in result[3]["objects"] if o["combatObjectArchetypeId"] == "combatobject.valtan.six-pizza.rock-pillar")
        self.assertEqual(obj["ownerHitChain"]["delayMs"], 1900)
        self.assertEqual(obj["hits"][0]["trigger"]["atMs"], 1820)

    def test_invalid_fields_and_owners_preserve_source(self):
        before = copy.deepcopy(self.docs[pipeline.COMBAT_AUTHORING_REL])
        for fields in (dict(atMs=1800), dict(atMs=600000), dict(outerRadiusM=-1), dict(pushRangeM=float("nan")),
                       dict(chainDelayMs=10), dict(stageId="missing"), dict(hitId="missing"), dict(knockdown=1)):
            with self.subTest(fields=fields), self.assertRaises(pipeline.PipelineError):
                self.apply(self.patch(**fields))
        self.assertEqual(before, self.docs[pipeline.COMBAT_AUTHORING_REL])


if __name__ == "__main__":
    unittest.main()
