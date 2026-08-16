from __future__ import annotations

import hashlib
import json
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[2]
MANIFEST_PATH = (
    ROOT
    / "Data/Effects/AuthoredCorrections/Artist/effect.artist.skill.31000.combo-role-manifest.json"
)
SOURCE_RECEIPT_PATH = ROOT / "Data/Effects/Imported/Artist/skill.31000.source-receipt.json"
FROZEN_ARTIST_F_PATH = (
    ROOT / "Data/Effects/Authored/effect.artist.skill.31470.unified.effect.json"
)
RESOURCES_ROOT = ROOT / "Client/Bin/Resources"


def load_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def sha256_file(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


class Artist31000ComboRoleManifestTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.manifest = load_json(MANIFEST_PATH)
        cls.receipt = load_json(SOURCE_RECEIPT_PATH)
        cls.documents: dict[str, dict] = {}
        for stage in cls.manifest["stages"]:
            name = f"{stage['effectAssetId']}.effect.json"
            cls.documents[stage["effectAssetId"]] = load_json(
                ROOT / "Data/Effects/Authored" / name
            )

    def test_source_receipt_and_package_are_pinned(self) -> None:
        evidence = self.manifest["sourceEvidence"]
        self.assertEqual(evidence["sourceReceiptSha256"], sha256_file(SOURCE_RECEIPT_PATH))
        source_package = next(
            row
            for row in self.receipt["sourcePackages"]
            if row["logicalPackage"] == evidence["logicalPackage"]
        )
        self.assertEqual(source_package["physicalPackage"], evidence["physicalPackage"])
        self.assertEqual(
            source_package["sourcePackageSha256"], evidence["sourcePackageSha256"]
        )

    def test_each_stage_is_an_exact_visible_suppressed_partition(self) -> None:
        expected_visible = {
            "effect.artist.skill.31000.ba1.unified": 3,
            "effect.artist.skill.31000.ba2.unified": 3,
            "effect.artist.skill.31000.ba3.unified": 5,
            "effect.artist.skill.31000.ba4.unified": 5,
        }
        for stage in self.manifest["stages"]:
            document = self.documents[stage["effectAssetId"]]
            elements = {row["id"]: row for row in document["elements"]}
            selected = {
                element_id
                for ids in stage["roles"].values()
                for element_id in ids
            }
            suppressed = set(stage["suppressedElementIds"])
            self.assertFalse(selected & suppressed)
            self.assertEqual(selected | suppressed, set(elements))
            self.assertEqual(len(elements), stage["documentElementCount"])
            self.assertEqual(len(selected), stage["visibleElementCount"])
            self.assertEqual(len(selected), expected_visible[stage["effectAssetId"]])
            self.assertEqual(
                {row["id"] for row in document["elements"] if row["visible"]},
                selected,
            )

    def test_selected_rows_share_only_the_proven_source_families(self) -> None:
        contract = self.manifest["selectedFamilyContract"]
        for stage in self.manifest["stages"]:
            elements = {
                row["id"]: row
                for row in self.documents[stage["effectAssetId"]]["elements"]
            }
            for role, ids in stage["roles"].items():
                for element_id in ids:
                    element = elements[element_id]
                    self.assertIn(
                        "|element:fx_pc_sdm_00.", element["sourceNode"].casefold()
                    )
                    if role in {
                        "BLACK_SLASH_MESH",
                        "TWO_SMALL_ONE_LARGE_SLASH_MESH",
                    }:
                        self.assertEqual(
                            element["sourceRecipe"]["rendererShape"], "mesh"
                        )
                        self.assertEqual(
                            element["material"]["sourceMaterialPath"],
                            contract["meshSourceMaterialPath"],
                        )
                        model = next(
                            binding["assetId"]
                            for binding in element["resources"]
                            if binding["slotId"] == "meshModel"
                        )
                        self.assertEqual(model, contract["meshAssetId"])
                    elif role == "BLACK_SLASH_EDGE":
                        self.assertEqual(
                            element["material"]["sourceMaterialPath"],
                            contract["edgeSourceMaterialPath"],
                        )
                    elif role == "INK_COMPANION":
                        self.assertEqual(
                            element["material"]["sourceMaterialPath"],
                            contract["companionSourceMaterialPath"],
                        )

    def test_ba4_slash_mesh_role_excludes_secondary_helix_and_decals(self) -> None:
        stage = next(
            row
            for row in self.manifest["stages"]
            if row["effectAssetId"] == "effect.artist.skill.31000.ba4.unified"
        )
        elements = {
            row["id"]: row
            for row in self.documents[stage["effectAssetId"]]["elements"]
        }
        slash_ids = set(stage["roles"]["TWO_SMALL_ONE_LARGE_SLASH_MESH"])
        self.assertEqual(len(slash_ids), 3)
        source_emitters = {
            elements[element_id]["sourceNode"].rsplit(
                "particlespriteemitter_", 1
            )[1]
            for element_id in slash_ids
        }
        self.assertEqual(source_emitters, {"20", "21", "22"})
        for element_id in stage["suppressedElementIds"]:
            element = elements[element_id]
            self.assertIs(element["visible"], False)
        self.assertIn("authored.source-particle.6892e2bbc1c244135f939f73", stage["suppressedElementIds"])
        self.assertIn("authored.source-particle.05fed8052067162678dd4153", stage["suppressedElementIds"])

    def test_all_selected_resource_bindings_are_physical(self) -> None:
        for stage in self.manifest["stages"]:
            document = self.documents[stage["effectAssetId"]]
            selected = {
                element_id
                for ids in stage["roles"].values()
                for element_id in ids
            }
            for element in document["elements"]:
                if element["id"] not in selected:
                    continue
                for binding in element["resources"]:
                    self.assertTrue(
                        (RESOURCES_ROOT / binding["assetId"]).is_file(),
                        binding["assetId"],
                    )
                for texture in (
                    element["material"].get("sourceProfile", {}).get("textures", [])
                ):
                    if texture["assetId"]:
                        self.assertTrue(
                            (RESOURCES_ROOT / texture["assetId"]).is_file(),
                            texture["assetId"],
                        )

    def test_artist_f_frozen_control_is_unchanged(self) -> None:
        self.assertEqual(
            sha256_file(FROZEN_ARTIST_F_PATH),
            "32676821df73c772bd313825c6968e2a79f9ada7af445b7734b07f0d40828799",
        )


if __name__ == "__main__":
    unittest.main()
