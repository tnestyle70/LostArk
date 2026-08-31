#!/usr/bin/env python3

from __future__ import annotations

from collections import Counter
import json
from pathlib import Path, PurePosixPath
import unittest


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
ASSET_ID = "effect.valtan.carrier-v1.attack.portal-rush.portal.clip-01"
AUTHORING_PATH = (
    "Effects/Authored/"
    "effect.valtan.carrier-v1.attack.portal-rush.portal.clip-01.effect.json"
)
AUTHORED_DOCUMENT_PATH = REPOSITORY_ROOT / "Data" / AUTHORING_PATH
CATALOG_PATH = REPOSITORY_ROOT / "Data/Effects/EffectCatalog.json"
CUE_PATH = (
    REPOSITORY_ROOT
    / "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
)
PATTERN_BINDING_PATH = (
    REPOSITORY_ROOT
    / "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
)

EXPECTED_ELEMENT_IDS = (
    "source.1b776620aaf4f38756f2",
    "source.1c9d5fc0aa230299b12a",
    "source.215a59ea8258f7e63813",
    "source.2d9b128f179cfb10a0ef",
    "source.516ebd8ab92481cddfb9",
    "source.542d45705d0ab10e26c4",
    "source.8bdb145e59f215ec3ca4",
    "source.905f8d81b798a5ba69d0",
    "source.b34062a699ad2c3749ec",
    "source.baf3b1214e15af4bdd50",
    "source.c15e35d8637a09e93cd6",
    "source.c8364a880f2d2bac929b",
    "source.cb18c0a3eb4715d141e5",
    "source.db4398c6dbd27ca0622f",
)

EXPECTED_SOURCE_NODES = (
    "valtan.source.1b776620aaf4f38756f22c5b99d7ae4913bb3ae42d2a0218e0b75b85fe556161",
    "valtan.source.1c9d5fc0aa230299b12a6060a14bf7955d196cc737bfe4f186a545f98d908fa0",
    "valtan.source.215a59ea8258f7e638131f567282fc36d2fbbe217ab486204278d7744f7ac2d0",
    "valtan.source.2d9b128f179cfb10a0ef1374073ff04e8843404470c13bf1816326d167211c5b",
    "valtan.source.516ebd8ab92481cddfb9e198854dd18d8076289ca8c1bde6164bb7a098859df3",
    "valtan.source.542d45705d0ab10e26c43226a64f0ac51d3b453832fe75ada5345b79dd6ac021",
    "valtan.source.8bdb145e59f215ec3ca4daa44118da82a3013f9fcd7bdcc6d6664f198b475730",
    "valtan.source.905f8d81b798a5ba69d086a4ac0d02e129b4cc429f8902baaea658f4c66660a1",
    "valtan.source.b34062a699ad2c3749ecbe57323dddc21be91d8e99363d6adfb7e469bac92117",
    "valtan.source.baf3b1214e15af4bdd50db885a8d3b7ba1f601975dfbaa1725ab5afb9ed39057",
    "valtan.source.c15e35d8637a09e93cd660eea4f2a8d3ecfc85ceb282edcb7dee7a75928ff952",
    "valtan.source.c8364a880f2d2bac929b13ddab6747bbc8bd994e5e299d11b0384871da2a68c7",
    "valtan.source.cb18c0a3eb4715d141e5e1fc63e6c56c2347e2505e853d38eb608d48ae3875f5",
    "valtan.source.db4398c6dbd27ca0622f37c5bacc6a864764a0d7ed4b25285cbf8b6ecfb5b058",
)

GROUP_01 = "fx_mn_rpbf_00_n.par_n_rpbf_potal_02_01"
GROUP_02 = "fx_mn_rpbf_00_n.par_n_rpbf_potal_02_02"


def load_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


class ValtanPortalRushOriginalElementsTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.document = load_json(AUTHORED_DOCUMENT_PATH)
        cls.catalog = load_json(CATALOG_PATH)
        cls.cues = load_json(CUE_PATH)
        cls.pattern_bindings = load_json(PATTERN_BINDING_PATH)

    def test_catalog_resolves_exact_direct_authored_document(self) -> None:
        rows = [
            row
            for row in self.catalog["effects"]
            if row.get("effectAssetId") == ASSET_ID
        ]
        self.assertEqual(1, len(rows))
        self.assertEqual("DIRECT_AUTHORED_DOCUMENT", rows[0]["payloadKind"])
        self.assertEqual(AUTHORING_PATH, rows[0]["authoringPath"])

    def test_portal_pattern_cue_and_animation_binding_are_exact(self) -> None:
        cues = [
            row
            for row in self.cues["cues"]
            if row.get("effectAssetId") == ASSET_ID
        ]
        self.assertEqual(1, len(cues))
        self.assertEqual("VALTAN_PORTAL_RUSH", cues[0]["patternId"])
        self.assertEqual("PORTAL", cues[0]["stageId"])
        self.assertEqual(
            "valtan.attack.portal-rush.portal", cues[0]["actionId"]
        )
        self.assertEqual(
            "valtan.attack.portal-rush.portal.clip.01",
            cues[0]["clipOccurrenceId"],
        )

        bindings = [
            row
            for row in self.pattern_bindings["bindings"]
            if row.get("actionId") == "valtan.attack.portal-rush.portal"
        ]
        self.assertEqual(1, len(bindings))
        self.assertEqual(1, len(bindings[0]["clips"]))
        clip = bindings[0]["clips"][0]
        self.assertEqual(
            "valtan.attack.portal-rush.portal.clip.01",
            clip["clipOccurrenceId"],
        )
        self.assertEqual("mesh_att_battle_18_01", clip["clip"])

    def test_original_fourteen_element_identity_and_order_are_preserved(self) -> None:
        self.assertEqual("lostark.effect-authoring", self.document["schema"])
        self.assertEqual(13, self.document["version"])
        self.assertEqual(ASSET_ID, self.document["effectAssetId"])

        elements = self.document["elements"]
        ids = tuple(row["id"] for row in elements)
        source_nodes = tuple(row["sourceNode"] for row in elements)
        self.assertEqual(EXPECTED_ELEMENT_IDS, ids)
        self.assertEqual(EXPECTED_SOURCE_NODES, source_nodes)
        self.assertEqual(len(ids), len(set(ids)))
        self.assertEqual(len(source_nodes), len(set(source_nodes)))

    def test_original_renderer_group_and_timing_contract_is_preserved(self) -> None:
        elements = self.document["elements"]
        self.assertEqual(
            Counter({"sprite": 11, "mesh": 3}),
            Counter(row["inventoryRendererShape"] for row in elements),
        )
        self.assertEqual(
            Counter({GROUP_01: 9, GROUP_02: 5}),
            Counter(row["groupId"] for row in elements),
        )

        for element in elements:
            with self.subTest(element_id=element["id"]):
                self.assertTrue(element["visible"])
                self.assertEqual("particle", element["kind"])
                self.assertTrue(element["sourceRecipe"]["enabled"])
                self.assertTrue(element["material"]["templateId"])
                timing = element["detail"]["timing"]
                if element["groupId"] == GROUP_01:
                    self.assertAlmostEqual(
                        0.8135589957237244, timing["startDelaySeconds"]
                    )
                    self.assertAlmostEqual(
                        0.18644100427627563, timing["lifeTimeSeconds"]
                    )
                else:
                    self.assertEqual(GROUP_02, element["groupId"])
                    self.assertAlmostEqual(
                        0.7503370046615601, timing["startDelaySeconds"]
                    )
                    self.assertAlmostEqual(
                        0.24966299533843994, timing["lifeTimeSeconds"]
                    )

    def test_resources_are_nonempty_and_resources_relative(self) -> None:
        resource_ids: set[str] = set()
        for element in self.document["elements"]:
            resources = element["resources"]
            self.assertGreater(len(resources), 0, element["id"])
            for resource in resources:
                asset_id = resource["assetId"]
                with self.subTest(element_id=element["id"], asset_id=asset_id):
                    self.assertTrue(asset_id.startswith("Effect/Valtan/"))
                    self.assertNotIn("\\", asset_id)
                    self.assertFalse(Path(asset_id).is_absolute())
                    self.assertFalse(len(asset_id) >= 2 and asset_id[1] == ":")
                    self.assertNotIn("..", PurePosixPath(asset_id).parts)
                    self.assertNotEqual("", resource["slotId"])
                resource_ids.add(asset_id)
        self.assertEqual(12, len(resource_ids))


if __name__ == "__main__":
    unittest.main(verbosity=2)
