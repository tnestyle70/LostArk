from __future__ import annotations

import hashlib
import json
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[2]
DOCUMENT_PATH = ROOT / "Data/Effects/Authored/effect.artist.skill.31460.unified.effect.json"
MANIFEST_PATH = (
    ROOT
    / "Data/Effects/AuthoredCorrections/Artist/effect.artist.skill.31460.role-manifest.json"
)
SOURCE_RECEIPT_PATH = (
    ROOT / "Data/Effects/Imported/Artist/CurrentCombat/skill.31460.source-receipt.json"
)
NORMALIZED_GRAPH_PATH = (
    ROOT
    / "Data/Effects/Imported/Artist/CurrentCombat/Graphs/skill.31460.normalized-effect-graph.json"
)
FROZEN_ARTIST_F_PATH = (
    ROOT / "Data/Effects/Authored/effect.artist.skill.31470.unified.effect.json"
)
RESOURCES_ROOT = ROOT / "Client/Bin/Resources"


def load_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def sha256_file(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def source_occurrence(element: dict) -> str:
    marker = "|element:"
    source_node = element["sourceNode"]
    if marker not in source_node:
        raise AssertionError(f"missing source occurrence identity: {element['id']}")
    return source_node.split(marker, 1)[1].casefold()


class Artist31460RoleManifestTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.document = load_json(DOCUMENT_PATH)
        cls.manifest = load_json(MANIFEST_PATH)
        cls.receipt = load_json(SOURCE_RECEIPT_PATH)
        cls.graph = load_json(NORMALIZED_GRAPH_PATH)
        cls.elements = {row["id"]: row for row in cls.document["elements"]}
        cls.role_ids = {
            role: set(ids) for role, ids in cls.manifest["target"]["roles"].items()
        }
        cls.selected = set().union(*cls.role_ids.values())
        cls.graph_paths = {
            f"{row['package']}.{row['objectPath']}".casefold()
            for row in cls.graph["nodes"]
        }

    def test_manifest_is_exact_disable_first_partition(self) -> None:
        self.assertEqual(self.manifest["schema"], "lostark.effect-role-manifest")
        self.assertEqual(self.manifest["version"], 1)
        self.assertEqual(self.manifest["effectAssetId"], self.document["effectAssetId"])
        self.assertEqual(len(self.document["elements"]), 82)
        self.assertEqual(len(self.selected), 18)
        self.assertTrue(self.selected <= set(self.elements))
        self.assertEqual(
            {row["id"] for row in self.document["elements"] if row["visible"]},
            self.selected,
        )
        self.assertEqual(
            sum(1 for row in self.document["elements"] if not row["visible"]), 64
        )
        self.assertEqual(len(self.role_ids["BUTTERFLY_BODY"]), 14)
        self.assertEqual(len(self.role_ids["BUTTERFLY_TRAIL"]), 2)
        self.assertEqual(len(self.role_ids["GROUND_INK_DECAL"]), 2)

    def test_selected_roles_keep_exact_source_occurrence_identity(self) -> None:
        for element_id in self.selected:
            occurrence = source_occurrence(self.elements[element_id])
            graph_occurrence = occurrence.split(".event_source-", 1)[0]
            self.assertIn(graph_occurrence, self.graph_paths)

        for element_id in self.role_ids["BUTTERFLY_BODY"]:
            row = self.elements[element_id]
            self.assertEqual(row["kind"], "particle")
            self.assertIn(
                row["material"]["sourceMaterialPath"],
                {
                    "fx_m_mi_o_00.fx_mi.fx_o_pa_spritewave_01_23_tr",
                    "fx_m_mi_o_00.fx_mi.fx_o_pa_spritewave_01_24_tr",
                    "fx_m_mi_o_00.fx_mi.fx_o_pa_spritewave_01_27_tr",
                },
            )
        for element_id in self.role_ids["GROUND_INK_DECAL"]:
            self.assertEqual(self.elements[element_id]["kind"], "decal")

    def test_source_evidence_and_repaired_dds_are_pinned(self) -> None:
        evidence = self.manifest["sourceEvidence"]
        self.assertEqual(evidence["sourceReceiptSha256"], sha256_file(SOURCE_RECEIPT_PATH))
        self.assertEqual(
            evidence["normalizedGraphSha256"], sha256_file(NORMALIZED_GRAPH_PATH)
        )
        source_package = next(
            row
            for row in self.receipt["sourcePackages"]
            if row["logicalPackage"] == evidence["logicalPackage"]
        )
        self.assertEqual(source_package["physicalPackage"], evidence["physicalPackage"])
        self.assertEqual(
            source_package["sourcePackageSha256"], evidence["sourcePackageSha256"]
        )
        for repair in self.manifest["sourceResourceRepairs"]:
            resource_path = RESOURCES_ROOT / repair["assetId"]
            self.assertTrue(resource_path.is_file(), repair["assetId"])
            self.assertEqual(sha256_file(resource_path), repair["sha256"])

    def test_spritewave_named_lanes_are_not_empty_or_generic_noise(self) -> None:
        expected = {
            "uv_noise_tex": "Effect/Artist/Textures/fx_k_auratile_02.dds",
            "maintex": "Effect/Artist/Textures/fx_a_line_003.dds",
        }
        typed_rows = [
            self.elements[element_id]
            for element_id in self.role_ids["BUTTERFLY_BODY"]
            if self.elements[element_id]["material"]["sourceMaterialPath"].endswith(
                ("_23_tr", "_24_tr")
            )
        ]
        self.assertEqual(len(typed_rows), 6)
        for row in typed_rows:
            textures = {
                texture["name"]: texture["assetId"]
                for texture in row["material"]["sourceProfile"]["textures"]
            }
            for name, asset_id in expected.items():
                self.assertEqual(textures[name], asset_id)
            expected_grass = (
                "Effect/Artist/Textures/fx_o_grass_03.dds"
                if row["material"]["sourceMaterialPath"].endswith("_24_tr")
                else "Effect/Artist/Textures/fx_o_grass_04.dds"
            )
            self.assertEqual(textures["dissolve_tex_01"], expected_grass)
            self.assertEqual(
                {binding["slotId"] for binding in row["resources"]},
                {"base", "noise", "dissolve", "emissive"},
            )

    def test_selected_lifetimes_are_bounded_role_windows(self) -> None:
        expected = {
            "authored.source-particle.a18001345de36223c9b2c4e0": 1.0,
            "authored.source-particle.c67bcfa2807a30511e45996e": 1.0,
            "authored.source-particle.cb346af47371feedccf9b652": 1.2,
            "authored.source-particle.32a4871cc934460404365309": 1.2,
            "authored.source-particle.ab175ceab43d84d23b8a9efc": 1.2,
            "authored.source-particle.a6b259e86343eae97f48c142": 1.2,
            "authored.source-particle.d7d5776483abfb036bc4e886": 1.5,
            "authored.source-particle.20736470f546e8c81108487a": 1.0,
            "authored.source-particle.c255fdc4c1172e15d387959b": 1.2,
            "authored.source-particle.1a843267c521e5037fc1271c": 1.0,
            "authored.source-particle.9d090bbe905a7daa6215e39c": 1.4,
            "authored.source-particle.64f6d0ab29d071e1a8d41dcb": 1.4,
            "authored.source-particle.8c510113f8256fd62b31de3b": 1.4,
            "authored.source-particle.4986a748ac0894912a52cc89": 1.4,
            "authored.source-particle.9180da6a19275da6a61130a9": 2.5,
            "authored.source-particle.9465e464a638f3123d24cd93": 2.5,
            "authored.source-decal.2f8ebdae3ea1e2a30fc91123": 1.5,
            "authored.source-decal.7139d7fc84cfc2aee0b40621": 1.5,
        }
        self.assertEqual(set(expected), self.selected)
        for element_id, seconds in expected.items():
            self.assertAlmostEqual(
                float(self.elements[element_id]["detail"]["timing"]["lifeTimeSeconds"]),
                seconds,
                places=7,
            )

    def test_artist_f_frozen_control_is_unchanged(self) -> None:
        self.assertEqual(
            sha256_file(FROZEN_ARTIST_F_PATH),
            "32676821df73c772bd313825c6968e2a79f9ada7af445b7734b07f0d40828799",
        )


if __name__ == "__main__":
    unittest.main()
