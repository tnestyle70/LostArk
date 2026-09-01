import json
import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]
EFFECT_TOOL_CPP = REPO_ROOT / "Client/Private/Effect_Tool.cpp"
EFFECT_TOOL_H = REPO_ROOT / "Client/Public/Effect_Tool.h"


def load_json(relative_path: str) -> dict:
    return json.loads((REPO_ROOT / relative_path).read_text(encoding="utf-8"))


def extract_function(source: str, signature: str) -> str:
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for index in range(brace, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
    raise AssertionError(f"unterminated function: {signature}")


class EffectToolElementOrderContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.cpp = EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        cls.header = EFFECT_TOOL_H.read_text(encoding="utf-8")

    def test_current_effect_exposes_typed_up_down_controls(self) -> None:
        render = extract_function(
            self.cpp,
            "void Client::CEffect_Tool::Render_ActiveAuthoredEffectTree()",
        )
        self.assertIn('ImGui::SmallButton("Up")', render)
        self.assertIn('ImGui::SmallButton("Down")', render)
        self.assertIn("Try_MoveSelectedElement(-1)", render)
        self.assertIn("Try_MoveSelectedElement(1)", render)
        self.assertIn("m_MarkedElementIds.empty()", render)
        self.assertIn("Has_UnappliedDetailDraft()", render)

    def test_move_preserves_stable_payload_and_commits_one_document(self) -> None:
        self.assertIn(
            "bool_t Try_MoveSelectedElement(int32_t iDirection);", self.header
        )
        move = extract_function(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_MoveSelectedElement(",
        )
        self.assertIn("Resolve_AuthoringFamily(*Selected)", move)
        self.assertGreaterEqual(move.count("Resolve_AuthoringFamily("), 3)
        self.assertIn("std::iter_swap(Selected, SwapWith);", move)
        self.assertIn("Try_CommitDocument(std::move(Staged))", move)
        self.assertLess(
            move.index("std::iter_swap(Selected, SwapWith);"),
            move.index("Try_CommitDocument(std::move(Staged))"),
        )
        self.assertNotRegex(move, r"\.strElementId\s*=(?!=)")

    def test_discard_reloads_saved_bytes_without_cross_owner_discard(self) -> None:
        session = extract_function(
            self.cpp, "void Client::CEffect_Tool::Render_AuthoringSessionBar()"
        )
        self.assertIn('ImGui::Button("Discard Changes...")', session)
        self.assertIn('"Discard Current Effect changes?"', session)
        self.assertIn("Try_ReloadActiveDocument(true)", session)

        reload_document = extract_function(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_ReloadActiveDocument(",
        )
        for foreign_dirty_owner in (
            "m_bOccurrenceTuningDirty",
            "m_bOccurrenceTransformDraftDirty",
            "m_bValtanAreaMapEffectDirty",
            "m_UnpublishedStaticAreaWorldDraft.has_value()",
        ):
            self.assertIn(foreign_dirty_owner, reload_document)
        self.assertIn("Try_LoadDocumentPathStaged(", reload_document)
        self.assertIn("bDiscardActiveDocumentDraft", reload_document)
        self.assertNotIn("Discard_ActiveDocument()", reload_document)


class ValtanSkyAxeRedTelegraphContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.effect_tool = EFFECT_TOOL_CPP.read_text(encoding="utf-8")

    def test_high_jump_owns_exact_direct_authored_effect(self) -> None:
        gameplay = load_json("Data/Valtan/Valtan.gameplay.json")
        pattern = next(
            row for row in gameplay["patterns"] if row["patternId"] == "VALTAN_HIGH_JUMP"
        )
        airborne = next(
            row for row in pattern["stages"] if row["stageId"] == "AIRBORNE"
        )
        spawn = next(
            row
            for row in airborne["events"]
            if row["eventId"]
            == "event.valtan.high-jump.airborne.spawn-target-axe"
        )
        self.assertEqual(
            spawn["combatObjectArchetypeId"],
            "combatobject.valtan.high-jump.target-axe",
        )
        self.assertEqual(spawn["volleyPolicy"], "PER_ALIVE_PLAYER")
        self.assertEqual(spawn["countPerResolvedTarget"], 1)
        self.assertEqual(
            spawn["spawnSchedule"],
            {
                "kind": "INTERVAL",
                "count": 3,
                "firstOffsetMs": 0,
                "intervalMs": 1333,
            },
        )

        combat_objects = load_json(
            "Data/Encounters/Valtan/ValtanCombatObjects.json"
        )
        combat_object = next(
            row
            for row in combat_objects["objects"]
            if row["combatObjectArchetypeId"]
            == "combatobject.valtan.high-jump.target-axe"
        )
        self.assertEqual(combat_object["ownerPatternId"], "VALTAN_HIGH_JUMP")

        bosses = load_json("Data/Actors/BossCatalog.json")
        valtan = next(
            row for row in bosses["bosses"] if row["archetypeId"] == "BOSS_VALTAN"
        )
        visual = next(
            row
            for row in valtan["combatObjectVisuals"]
            if row["combatObjectArchetypeId"]
            == "combatobject.valtan.high-jump.target-axe"
        )
        self.assertEqual(visual["effectAssetId"], "effect.valtan.sky-axe.active")

        catalog = load_json("Data/Effects/EffectCatalog.json")
        catalog_row = next(
            row
            for row in catalog["effects"]
            if row["effectAssetId"] == "effect.valtan.sky-axe.active"
        )
        self.assertEqual(catalog_row["payloadKind"], "DIRECT_AUTHORED_DOCUMENT")
        self.assertEqual(
            catalog_row["authoringPath"],
            "Effects/Authored/effect.valtan.sky-axe.active.effect.json",
        )

        presentation = load_json("Data/Valtan/Valtan.presentation.json")
        independent_rows = [
            row
            for row in presentation["independentEffects"]
            if row["independentEffectId"]
            == "valtan.independent-effect.target-axe"
        ]
        self.assertEqual(1, len(independent_rows))
        self.assertEqual(
            "event.valtan.high-jump.airborne.spawn-target-axe",
            independent_rows[0]["spawnEventId"],
        )

        product_cues = load_json(
            "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
        )["cues"]
        sky_axe_product_cues = [
            row
            for row in product_cues
            if row["effectAssetId"] == "effect.valtan.sky-axe.active"
        ]
        self.assertEqual(4, len(sky_axe_product_cues))
        self.assertEqual(
            {"VALTAN_TERRAIN_DESTRUCTION"},
            {row["patternId"] for row in sky_axe_product_cues},
        )
        self.assertEqual(
            0,
            sum(
                row["patternId"] == "VALTAN_HIGH_JUMP"
                for row in sky_axe_product_cues
            ),
            "the three target-axe plays come from the one Server combat-object "
            "schedule, not duplicate Pattern cue owners",
        )

    def test_user_deleted_independent_red_floor_stays_absent(self) -> None:
        authored = load_json(
            "Data/Effects/Authored/effect.valtan.sky-axe.active.effect.json"
        )
        deleted_element_id = (
            "authored.copy.authored.copy.authored.copy.authored.copy."
            "authored.copy.sprite_particle_8.1.1.2.1.1"
        )
        self.assertNotIn(
            deleted_element_id,
            {row["id"] for row in authored["elements"]},
        )

    def test_combat_object_product_save_hot_reloads_without_a_pattern_cue(self) -> None:
        refresh_index = extract_function(
            self.effect_tool,
            "bool_t Client::CEffect_Tool::Refresh_DirectAuthoredEditableIndex(",
        )
        compact_refresh_index = "".join(refresh_index.split())
        self.assertIn(
            "++StagedBossProductCueMappingCounts[Visual.effectAssetId];",
            compact_refresh_index,
        )
        self.assertIn(
            "++StagedBossProductCueMappingCounts[Visual.hitEffectAssetId];",
            compact_refresh_index,
        )

        save = extract_function(
            self.effect_tool,
            "bool_t Client::CEffect_Tool::Try_SaveDocument()",
        )
        registered_product = (
            "constbool_tbRegisteredDirectProduct="
            "CEffectCatalog::Is_DirectAuthoredDocument("
            "m_ActiveDocument->strEffectAssetId);"
        )
        self.assertIn(registered_product, "".join(save.split()))
        self.assertIn(
            "CEffectPresentationService::Reload_SelectedProductEffect(", save
        )

    def test_every_sky_axe_resource_exists_physically(self) -> None:
        authored = load_json(
            "Data/Effects/Authored/effect.valtan.sky-axe.active.effect.json"
        )
        resources_root = REPO_ROOT / "Client/Bin/Resources"
        missing = []
        for element in authored["elements"]:
            for resource in element.get("resources", []):
                path = resources_root / Path(resource["assetId"])
                if not path.is_file() or path.stat().st_size == 0:
                    missing.append(f"{element['id']}:{resource['assetId']}")
        self.assertEqual(missing, [])


if __name__ == "__main__":
    unittest.main()
