"""Guardian creation data must preserve native hair and allow independent outfits."""

import itertools
import json
from pathlib import Path
import unittest


REPO_ROOT = Path(__file__).resolve().parents[2]
RESOURCE_ROOT = REPO_ROOT / "Client/Bin/Resources"
GUARDIAN_LOOKS = (
    "class_select_hr00",
    "original_00",
    "source_ddk_01",
    "source_ddk_02",
    "source_ddk_03",
)


def load_json(relative_path):
    return json.loads((REPO_ROOT / relative_path).read_text(encoding="utf-8"))


class GuardianCustomizingHairTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        catalog = load_json("Data/Actors/EquipmentPresentationCatalog.json")
        cls.sets = {item["visualSetId"]: item for item in catalog["visualSets"]}
        cls.hair = load_json("Data/UI/Customizing/CustomizingHairstyles.json")[
            "classes"
        ]["GuardianKnight"]
        cls.costumes = load_json("Data/UI/Customizing/CustomizingCostumes.json")[
            "classes"
        ]["GuardianKnight"]["costume"]
        cls.character = next(
            item
            for item in load_json("Data/Actors/CharacterCatalog.json")["characters"]
            if item["assetId"] == "GuardianKnight"
        )

    def test_creation_uses_body_hair_without_reindexing_existing_hair_choices(self):
        self.assertIs(self.hair.get("defaultBodyHair"), True)
        self.assertNotIn("defaultVisualSetId", self.hair)
        entries = self.hair["hairstyle"]
        self.assertEqual(list(range(51)), [entry["index"] for entry in entries])
        self.assertEqual("PC_DK_55_Hair", entries[0]["sourceHairId"])
        self.assertEqual("PC_DK_56_Hair", entries[1]["sourceHairId"])

    def test_customizing_variants_preserve_original_outfit_parts_and_slots(self):
        self.assertEqual(list(range(5)), [entry["objectUnit"] for entry in self.costumes])
        for look, entry in zip(GUARDIAN_LOOKS, self.costumes):
            with self.subTest(look=look):
                prefix = f"character.guardian_knight.{look}"
                self.assertEqual(prefix + ".customizing_outfit", entry["visualSetId"])
                original = self.sets[prefix + ".outfit"]
                variant = self.sets[entry["visualSetId"]]
                self.assertEqual("GUARDIANKNIGHT", variant["classId"])
                self.assertIn("HEAD", original["occupiedSlots"])
                self.assertEqual(1, sum(p["partRole"] == "HEAD" for p in original["parts"]))
                self.assertEqual(
                    [slot for slot in original["occupiedSlots"] if slot != "HEAD"],
                    variant["occupiedSlots"],
                )
                self.assertEqual(
                    [part for part in original["parts"] if part["partRole"] != "HEAD"],
                    variant["parts"],
                )
                for field in ("classId", "categoryId", "catalogStatus", "primarySlot"):
                    self.assertEqual(original[field], variant[field])

    def test_every_available_hair_and_costume_have_no_conflict_in_either_order(self):
        available_hair = [
            self.sets[entry["visualSetId"]]
            for entry in self.hair["hairstyle"]
            if entry["visualSetId"] in self.sets
        ]
        self.assertGreater(len(available_hair), 0)
        for entry, hair in itertools.product(self.costumes, available_hair):
            costume = self.sets[entry["visualSetId"]]
            self.assertEqual("HEAD", hair["primarySlot"])
            self.assertTrue(hair["parts"])
            for first, second in ((costume, hair), (hair, costume)):
                with self.subTest(first=first["visualSetId"], second=second["visualSetId"]):
                    # The wear API replaces an entire conflicting set. Disjoint coverage
                    # is the data contract that keeps both sets independent in either order.
                    self.assertTrue(set(first["occupiedSlots"]).isdisjoint(second["occupiedSlots"]))
                    self.assertNotEqual(first["primarySlot"], second["primarySlot"])

    def test_original_body_hair_keeps_its_native_material_and_authored_color(self):
        self.assertEqual("Character/GuardianKnight/GuardianKnight.wmodel", self.character["bodyModel"])
        hair = [
            item
            for item in self.character["modelMaterialOverrides"]
            if item["modelAssetId"] == self.character["bodyModel"]
            and item["materialName"] == "pc_ft_15_hair_mi"
        ]
        self.assertEqual(1, len(hair))
        self.assertEqual("source.character.hair-ddk.v1", hair[0]["family"])
        self.assertEqual(
            [1.0, 0.839, 0.863, 1.0],
            hair[0]["parameters"]["var_base_haircolor_base_ui"],
        )
        self.assertEqual(
            [0.658, 0.638, 0.584, 1.0],
            hair[0]["parameters"]["var_base_hairtwotonecolor_ui"],
        )

    @unittest.skipUnless((RESOURCE_ROOT / "Character/GuardianKnight").is_dir(),
                         "installed Guardian Resources are not present on this checkout")
    def test_installed_models_and_native_hair_textures_exist(self):
        assets = {self.character["bodyModel"]}
        for entry in self.costumes + self.hair["hairstyle"]:
            visual_set = self.sets.get(entry["visualSetId"])
            if visual_set is not None:
                assets.update(part["modelAssetId"] for part in visual_set["parts"])
        for override in self.character["modelMaterialOverrides"]:
            if override["materialName"] == "pc_ft_15_hair_mi":
                assets.update(texture["assetId"] for texture in override["textures"])
        for asset in sorted(assets):
            with self.subTest(asset=asset):
                self.assertTrue((RESOURCE_ROOT / asset).is_file(), asset)
                self.assertGreater((RESOURCE_ROOT / asset).stat().st_size, 0, asset)


if __name__ == "__main__":
    unittest.main()
