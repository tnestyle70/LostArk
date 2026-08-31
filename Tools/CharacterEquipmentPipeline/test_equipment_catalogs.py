import json
from pathlib import Path, PurePosixPath
import unittest


REPO_ROOT = Path(__file__).resolve().parents[2]
CATALOG_PATH = REPO_ROOT / "Data" / "Actors" / "EquipmentPresentationCatalog.json"
PRESET_PATH = REPO_ROOT / "Data" / "Actors" / "EquipmentLoadoutPresets.json"
SELECTION_PATH = (
    REPO_ROOT
    / "Tools"
    / "CharacterEquipmentPipeline"
    / "RepresentativeCharacterEquipment.json"
)
SLOTS = ["HEAD", "SHOULDER", "UPPER", "LOWER", "HANDS", "WEAPON"]
CLASSES = {
    "LANCE_MASTER",
    "GUNSLINGER",
    "SLAYER",
    "ARTIST",
    "DIMENSIONMASTER",
    "WARLORD",
}


def load_json(path: Path):
    return json.loads(path.read_text(encoding="utf-8"))


class EquipmentCatalogTests(unittest.TestCase):
    def setUp(self):
        self.catalog = load_json(CATALOG_PATH)
        self.presets = load_json(PRESET_PATH)
        self.selection = load_json(SELECTION_PATH)

    def test_catalog_matches_representative_selection(self):
        self.assertEqual("lostark.equipment-presentation-catalog", self.catalog["schema"])
        self.assertEqual(1, self.catalog["formatVersion"])
        self.assertEqual(SLOTS, self.catalog["slotIds"])

        visual_sets = self.catalog["visualSets"]
        self.assertEqual(12, len(visual_sets))
        self.assertEqual(17, sum(len(item["parts"]) for item in visual_sets))
        self.assertEqual(CLASSES, {item["classId"] for item in visual_sets})

        selected_by_id = {
            item["visualSetId"]: item for item in self.selection["sets"]
        }
        self.assertEqual(set(selected_by_id), {item["visualSetId"] for item in visual_sets})
        for visual_set in visual_sets:
            source = selected_by_id[visual_set["visualSetId"]]
            self.assertEqual(source["classId"], visual_set["classId"])
            self.assertEqual(source["catalogStatus"], visual_set["catalogStatus"])
            self.assertEqual(source["primarySlot"], visual_set["primarySlot"])
            self.assertEqual(source["coverageSlots"], visual_set["occupiedSlots"])
            self.assertIn(visual_set["primarySlot"], visual_set["occupiedSlots"])
            source_parts = {item["partId"]: item for item in source["parts"]}
            self.assertEqual(set(source_parts), {item["partId"] for item in visual_set["parts"]})
            for part in visual_set["parts"]:
                self.assertEqual(
                    source_parts[part["partId"]]["targetModelAssetId"],
                    part["modelAssetId"],
                )
                path = PurePosixPath(part["modelAssetId"])
                self.assertFalse(path.is_absolute())
                self.assertNotIn("..", path.parts)
                self.assertEqual("Character", path.parts[0])
                self.assertEqual("Equipment", path.parts[2])

    def test_socket_stance_and_hidden_mesh_contracts_are_exact(self):
        parts = {
            (visual_set["visualSetId"], part["partId"]): part
            for visual_set in self.catalog["visualSets"]
            for part in visual_set["parts"]
        }
        expected_socket_contract = {
            ("character.lance_master.wp_wflm_00", "long_spear"): (
                "b_weapon_rhand",
                0.0,
                "LANCE_MASTER_LONG_SPEAR",
            ),
            ("character.lance_master.wp_wflm_00", "short_spear"): (
                "b_weapon_rhand",
                0.0,
                "LANCE_MASTER_SHORT_SPEAR",
            ),
            ("character.slayer.wp_wwbk_03", "main"): (
                "b_weapon_rhand",
                0.0,
                None,
            ),
            ("character.artist.wp_wsdm_09", "main"): ("b_wp_1", 0.0, None),
            ("character.warlord.wp_wwgl_04", "main"): (
                "b_weapon_rhand",
                0.0,
                None,
            ),
            ("character.warlord.wp_wwgl_04", "shield"): (
                "b_weapon_lhand",
                0.0,
                None,
            ),
            ("character.dimensionmaster.wp_wswp_m_06", "l"): (
                "b_wp_swm_m_1",
                0.0,
                None,
            ),
            ("character.dimensionmaster.wp_wswp_m_06", "s"): (
                "b_wp_swm_m_2",
                0.0,
                None,
            ),
            ("character.dimensionmaster.wp_wswp_m_06", "p"): (
                "b_wp_swm_m_3",
                0.0,
                None,
            ),
            ("character.dimensionmaster.wp_wswp_m_06", "e"): (
                "b_wp_swm_m_4_02",
                180.0,
                None,
            ),
        }
        socketed = {key: value for key, value in parts.items() if value["attachmentMode"] == "SOCKETED"}
        self.assertEqual(set(expected_socket_contract), set(socketed))
        for key, expected in expected_socket_contract.items():
            part = parts[key]
            self.assertEqual(expected, (
                part["socketBoneId"],
                part["socketYawDegrees"],
                part["requiredStance"],
            ))
        for part in parts.values():
            if part["attachmentMode"] == "SKINNED":
                self.assertIsNone(part["socketBoneId"])
                self.assertEqual(0.0, part["socketYawDegrees"])
                self.assertIsNone(part["requiredStance"])
        artist_head = parts[("character.artist.default_variant_01.head", "head")]
        self.assertEqual(2, artist_head["hiddenMeshMask"])
        self.assertTrue(
            all(
                part["hiddenMeshMask"] == (2 if part is artist_head else 0)
                for part in parts.values()
            )
        )

    def test_presets_are_authoring_only_and_reference_primary_slots_without_overlap(self):
        self.assertEqual("lostark.equipment-loadout-presets", self.presets["schema"])
        self.assertEqual(1, self.presets["formatVersion"])
        self.assertIs(True, self.presets["authoringOnly"])
        self.assertEqual(SLOTS, self.presets["slots"])
        class_presets = self.presets["classPresets"]
        self.assertEqual(CLASSES, {item["classId"] for item in class_presets})
        self.assertEqual(6, len(class_presets))

        sets_by_id = {
            visual_set["visualSetId"]: visual_set
            for visual_set in self.catalog["visualSets"]
        }
        for preset in class_presets:
            selections = preset["slotSelections"]
            self.assertEqual(set(SLOTS), set(selections))
            occupied = set()
            for slot in SLOTS:
                visual_set_id = selections[slot]
                if visual_set_id is None:
                    continue
                self.assertIn(visual_set_id, sets_by_id)
                visual_set = sets_by_id[visual_set_id]
                self.assertEqual(preset["classId"], visual_set["classId"])
                self.assertEqual(slot, visual_set["primarySlot"])
                overlap = occupied.intersection(visual_set["occupiedSlots"])
                self.assertFalse(overlap, f"Preset slot overlap: {overlap}")
                occupied.update(visual_set["occupiedSlots"])


if __name__ == "__main__":
    unittest.main()
