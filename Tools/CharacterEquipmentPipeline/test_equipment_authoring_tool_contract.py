import pathlib
import re
import unittest
import xml.etree.ElementTree as ET


ROOT = pathlib.Path(__file__).resolve().parents[2]
MAIN_H = ROOT / "Client" / "Public" / "MainApp.h"
MAIN_CPP = ROOT / "Client" / "Private" / "MainApp.cpp"
PREVIEW_H = ROOT / "Client" / "Public" / "CharacterPreviewPanel.h"
TOOL_H = ROOT / "Client" / "Public" / "EquipmentAuthoringTool.h"
TOOL_CPP = ROOT / "Client" / "Private" / "EquipmentAuthoringTool.cpp"
PROJECT = ROOT / "Client" / "Default" / "Client.vcxproj"
FILTERS = ROOT / "Client" / "Default" / "Client.vcxproj.filters"
CHARACTER_SPEC_H = ROOT / "Client" / "Public" / "CharacterSpec.h"
CHARACTER_CPP = ROOT / "Client" / "Private" / "Character.cpp"
PRESENTATION_SERVICE_CPP = (
    ROOT / "Client" / "Private" / "EquipmentPresentationService.cpp"
)
LOGIC_CPP = {
    name: ROOT / "Client" / "Private" / f"Logic_{name}.cpp"
    for name in (
        "LanceMaster", "GunSlinger", "Slayer", "Artist",
        "DimensionMaster", "Warlord",
    )
}


class EquipmentAuthoringToolContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.main_h = MAIN_H.read_text(encoding="utf-8-sig")
        cls.main_cpp = MAIN_CPP.read_text(encoding="utf-8-sig")
        cls.preview_h = PREVIEW_H.read_text(encoding="utf-8-sig")
        cls.tool_h = TOOL_H.read_text(encoding="utf-8-sig")
        cls.tool_cpp = TOOL_CPP.read_text(encoding="utf-8-sig")
        cls.project = PROJECT.read_text(encoding="utf-8-sig")
        cls.filters = FILTERS.read_text(encoding="utf-8-sig")
        cls.character_spec_h = CHARACTER_SPEC_H.read_text(encoding="utf-8-sig")
        cls.character_cpp = CHARACTER_CPP.read_text(encoding="utf-8-sig")
        cls.presentation_service_cpp = PRESENTATION_SERVICE_CPP.read_text(
            encoding="utf-8-sig"
        )
        cls.logic_cpp = {
            name: path.read_text(encoding="utf-8-sig")
            for name, path in LOGIC_CPP.items()
        }

    def test_f1_hub_owns_lazy_lifecycle_and_preview_input_gate(self):
        for token in (
            "DEBUG_TOOL::EQUIPMENT",
            'toolCell("Equipment Authoring Tool", DEBUG_TOOL::EQUIPMENT)',
            "make_unique<CEquipmentAuthoringTool>",
            "m_pEquipmentAuthoringTool->Render(\n\t\t\t\t\tDEBUG_TOOL::EQUIPMENT == m_eDebugInputOwner)",
            "m_pEquipmentAuthoringTool->On_LevelChanged()",
            "m_pEquipmentAuthoringTool.reset()",
        ):
            self.assertIn(token, self.main_cpp)
        self.assertIn("EQUIPMENT,", self.main_h)
        self.assertIn("unique_ptr<CEquipmentAuthoringTool>", self.main_h)

    def test_no_new_function_key_or_transport_database_boundary(self):
        for forbidden_key in (
            "VK_F2", "VK_F3", "VK_F4", "VK_F5",
            "VK_F7", "VK_F8", "VK_F9", "VK_F10", "VK_F11", "VK_F12",
        ):
            self.assertNotIn(forbidden_key, self.tool_cpp)
        for forbidden_call in (
            '#include "NetworkManager.h"',
            "CNetworkManager::",
            "C2S_",
            "sqlite3",
            "CharacterState.sqlite3",
        ):
            self.assertNotIn(forbidden_call, self.tool_cpp)

    def test_six_classes_six_slots_and_required_actions_are_stable(self):
        for class_id in (
            "LANCE_MASTER", "GUNSLINGER", "SLAYER", "ARTIST",
            "DIMENSIONMASTER", "WARLORD",
        ):
            self.assertIn(f'"{class_id}"', self.tool_cpp)
        for asset_id in (
            "LanceMaster", "GunSlinger", "Slayer", "Artist",
            "DimensionMaster", "Warlord",
        ):
            self.assertIn(f'"{asset_id}"', self.tool_cpp)
        for action in (
            "Equip Selected", "Unequip Slot", "Unequip All",
            "Reset Character Default", "Reload Catalog",
            "Save Presets", "Reload Presets",
        ):
            self.assertIn(f'"{action}"', self.tool_cpp)
        self.assertIn("SLOT_COUNT = ETOI(EQUIPMENT_SLOT_ID::END)", self.tool_h)

    def test_compound_slot_conflicts_clear_primary_selection_transactionally(self):
        self.assertIn("Set_OccupiedMask(*pCandidate)", self.tool_cpp)
        self.assertIn("candidateMask & Set_OccupiedMask(*pEquipped)", self.tool_cpp)
        self.assertIn("staged[index].clear()", self.tool_cpp)
        self.assertIn("Find_SetOccupyingSlot(", self.tool_cpp)
        self.assertIn("staged[primarySlotIndex].clear()", self.tool_cpp)
        self.assertLess(
            self.tool_cpp.index("m_PresentationService.Apply_Preview(\n\t\t*character, m_Catalog, stagedLoadout"),
            self.tool_cpp.index("m_Loadouts[m_iSelectedClass] = stagedLoadout"),
        )

    def test_dirty_draft_uses_shared_preview_lock_owner(self):
        self.assertIn("EQUIPMENT_TOOL,", self.preview_h)
        self.assertIn(
            "CHARACTER_PREVIEW_LOCK_OWNER::EQUIPMENT_TOOL", self.tool_cpp
        )
        self.assertIn("m_pPreviewPanel->Render_Selector(", self.tool_cpp)
        self.assertIn("false);", self.tool_cpp)

    def test_preset_save_is_preview_only_reparsed_and_atomic(self):
        for token in (
            "lostark.equipment-loadout-presets",
            '"  \\"authoringOnly\\": true,\\n"',
            '"slotSelections"',
            "Make_UniqueTemporaryPath",
            "Load_PresetsFromPath(temporary, reparsed, error)",
            "MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH",
            "Validate_Loadouts(staged, m_Catalog, outError)",
        ):
            self.assertIn(token, self.tool_cpp)

    def test_preset_reload_applies_staged_visual_before_document_commit(self):
        start = self.tool_cpp.index(
            "bool_t Client::CEquipmentAuthoringTool::Reload_Presets()"
        )
        end = self.tool_cpp.index(
            "bool_t Client::CEquipmentAuthoringTool::Save_Presets()", start
        )
        body = self.tool_cpp[start:end]
        parse_at = body.index("Load_PresetsFromPath(Preset_Path(), staged, error)")
        apply_at = body.index(
            "m_PresentationService.Apply_Preview(\n\t\t\t*previewCharacter, m_Catalog,"
        )
        loadout_commit_at = body.index("m_Loadouts = std::move(staged)")
        dirty_commit_at = body.index("Set_Dirty(false)")
        self.assertLess(parse_at, apply_at)
        self.assertLess(apply_at, loadout_commit_at)
        self.assertLess(loadout_commit_at, dirty_commit_at)
        self.assertIn(
            "previous UI loadout, visual, and dirty state were preserved", body
        )
        self.assertIn(
            "if (!m_bPreviewInputEnabled)\n\t\t{", body
        )
        self.assertIn(
            "presets committed; they will apply when a playable class preview is selected",
            body,
        )

    def test_catalog_reload_applies_staged_generation_before_catalog_commit(self):
        start = self.tool_cpp.index(
            "bool_t Client::CEquipmentAuthoringTool::Reload_Catalog()"
        )
        end = self.tool_cpp.index(
            "bool_t Client::CEquipmentAuthoringTool::Reload_Presets()", start
        )
        body = self.tool_cpp[start:end]
        load_at = body.index("stagedCatalog.Load(error)")
        validate_at = body.index("Validate_Loadouts(m_Loadouts, stagedCatalog, error)")
        apply_at = body.index(
            "m_PresentationService.Apply_Preview(\n\t\t\t*previewCharacter, stagedCatalog,"
        )
        catalog_commit_at = body.index("m_Catalog = std::move(stagedCatalog)")
        self.assertLess(load_at, validate_at)
        self.assertLess(validate_at, apply_at)
        self.assertLess(apply_at, catalog_commit_at)
        self.assertIn(
            "m_bCatalogLoaded && hasActivePlayablePreview", body
        )
        self.assertNotIn("m_bPresetsLoaded && hasActivePlayablePreview", body)
        self.assertIn(
            "previous catalog, diagnostics, visual, and dirty draft were preserved",
            body,
        )
        self.assertIn(
            "catalog committed; no active equipment preview required a visual refresh",
            body,
        )

    def test_preview_mutations_require_an_active_playable_preview_clone(self):
        self.assertIn("Resolve_ActivePlayablePreview(", self.tool_h)
        helper_start = self.tool_cpp.index(
            "bool_t Client::CEquipmentAuthoringTool::Resolve_ActivePlayablePreview("
        )
        helper_end = self.tool_cpp.index(
            "void Client::CEquipmentAuthoringTool::Set_Dirty", helper_start
        )
        helper = self.tool_cpp[helper_start:helper_end]
        self.assertIn("m_pPreviewPanel->Is_PreviewActive()", helper)
        self.assertIn("Find_ClassIndexByAssetId(", helper)
        self.assertIn("CAnimationTargetService::Resolve_Character()", helper)
        self.assertNotIn("Resolve_SceneCharacter", self.tool_cpp)

    def test_project_and_filters_register_all_new_contract_files(self):
        for relative in (
            r"..\Public\EquipmentAuthoringTool.h",
            r"..\Public\EquipmentPresentationCatalog.h",
            r"..\Public\EquipmentPresentationService.h",
            r"..\Private\EquipmentAuthoringTool.cpp",
            r"..\Private\EquipmentPresentationCatalog.cpp",
            r"..\Private\EquipmentPresentationService.cpp",
            r"..\..\Data\Actors\EquipmentLoadoutPresets.json",
            r"..\..\Data\Actors\EquipmentPresentationCatalog.json",
        ):
            self.assertEqual(1, self.project.count(relative), relative)
            self.assertEqual(1, self.filters.count(relative), relative)
        ET.parse(PROJECT)
        ET.parse(FILTERS)

    def test_default_parts_have_authored_preview_slots_and_runtime_uses_mask(self):
        self.assertIn("enum class EQUIPMENT_PRESENTATION_SLOT", self.character_spec_h)
        self.assertIn("ePresentationSlot", self.character_spec_h)

        expected_slots = {
            "LanceMaster": {
                "HEAD": 2, "SHOULDER": 1, "UPPER": 2,
                "LOWER": 1, "HANDS": 1,
            },
            "GunSlinger": {
                "HEAD": 1, "SHOULDER": 1, "UPPER": 1,
                "LOWER": 1, "HANDS": 1,
            },
            "Slayer": {
                "HEAD": 1, "SHOULDER": 1, "UPPER": 1,
                "LOWER": 1, "HANDS": 1,
            },
            "Artist": {
                "HEAD": 1, "SHOULDER": 1, "UPPER": 1,
                "LOWER": 1, "HANDS": 1,
            },
            "Warlord": {
                "HEAD": 2, "SHOULDER": 1, "UPPER": 1,
                "LOWER": 1, "HANDS": 1,
            },
        }
        for class_name, counts in expected_slots.items():
            match = re.search(
                r"constexpr EQUIPMENT_PART_SPEC Equipment\[\]\s*=\s*\{(.*?)\n\s*\};",
                self.logic_cpp[class_name],
                re.DOTALL,
            )
            self.assertIsNotNone(match, class_name)
            equipment_block = match.group(1)
            self.assertEqual(
                sum(counts.values()),
                equipment_block.count("EQUIPMENT_PRESENTATION_SLOT::"),
                class_name,
            )
            for slot, count in counts.items():
                self.assertEqual(
                    count,
                    equipment_block.count(f"EQUIPMENT_PRESENTATION_SLOT::{slot}"),
                    f"{class_name}:{slot}",
                )

        dimension_master = self.logic_cpp["DimensionMaster"]
        self.assertNotIn("constexpr EQUIPMENT_PART_SPEC Equipment[]", dimension_master)
        self.assertNotIn("size(Equipment)", dimension_master)

        for catalog_slot in (
            "HEAD", "SHOULDER", "UPPER", "LOWER", "HANDS", "WEAPON",
        ):
            self.assertIn(
                f"case EQUIPMENT_SLOT_ID::{catalog_slot}:",
                self.presentation_service_cpp,
            )
        self.assertIn(
            "previewParts, occupiedSlotsMask, outError",
            self.presentation_service_cpp,
        )
        self.assertIn(
            "Apply_DefaultEquipmentVisibility(occupiedSlotsMask)",
            self.character_cpp,
        )
        self.assertIn(
            "EquipmentPresentationSlotMask(equipment.ePresentationSlot)",
            self.character_cpp,
        )
        self.assertNotIn("pBody->Set_HiddenMeshes(0u)", self.character_cpp)


if __name__ == "__main__":
    unittest.main()
