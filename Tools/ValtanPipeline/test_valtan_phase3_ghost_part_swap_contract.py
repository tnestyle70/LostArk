import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8")


def function_slice(source: str, signature: str, next_signature: str) -> str:
    begin = source.index(signature)
    end = source.index(next_signature, begin)
    return source[begin:end]


class ValtanPhase3GhostPartSwapContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.valtan_cpp = read("Client/Private/Valtan.cpp")
        cls.valtan_h = read("Client/Public/Valtan.h")
        cls.boss_catalog = json.loads(read("Data/Actors/BossCatalog.json"))

    def test_phase_three_changes_only_the_primary_presentation_identity(self) -> None:
        apply_state = function_slice(
            self.valtan_cpp,
            "bool_t CValtan::Apply_BossCombatState(",
            "bool_t CValtan::Apply_BrokenArmorMask(",
        )
        for token in (
            '"BOSS_VALTAN" == m_strArchetypeId',
            "INVALID_NET_ENTITY_ID == m_iOwnerBossNetEntityId",
            "state.iGameplayPhase >= 3u",
            '"BOSS_VALTAN_GHOST" : "BOSS_VALTAN"',
            "Replace_PresentationPartGroup(",
            "m_BossCombatState = state;",
        ):
            self.assertIn(token, apply_state)
        self.assertNotIn("m_strArchetypeId.assign", apply_state)
        self.assertNotIn("m_strArchetypeId =", apply_state)
        self.assertLess(
            apply_state.index("Replace_PresentationPartGroup("),
            apply_state.index("m_BossCombatState = state;"),
            "a missing visual must be isolated without rejecting the valid combat snapshot",
        )
        self.assertIn("m_strPresentationPartArchetypeId", self.valtan_h)

    def test_body_weapon_and_armour_publish_through_one_group_swap(self) -> None:
        replace = function_slice(
            self.valtan_cpp,
            "bool_t CValtan::Replace_PresentationPartGroup(",
            "/* The shoulder and arm plates are authored on the body rig",
        )
        for token in (
            "PART_OBJECT_MAP StagedParts;",
            'TEXT("Prototype_GameObject_Body_Valtan")',
            'TEXT("Prototype_GameObject_Part_Equipment")',
            "for (const BOSS_ARMOR_PART_ENTRY& armorPart",
            'Replace_PartObjectGroup(\n\t\t\tTEXT("Part_"), std::move(StagedParts))',
            "m_pBodyModelCom = StagedBodyModel;",
            "m_pBodyVisualRootCom = StagedBodyVisualRoot;",
            "m_ArmorPartTagsByStateMask = std::move(StagedArmorPartTags);",
        ):
            self.assertIn(token, replace)

        commit = replace.index("Replace_PartObjectGroup(")
        before_commit = replace[:commit]
        for live_assignment in (
            "m_pBodyModelCom =",
            "m_pBodyVisualRootCom =",
            "m_ArmorPartTagsByStateMask =",
            "m_strPresentationPartArchetypeId =\n\t\tstd::move",
        ):
            self.assertNotIn(live_assignment, before_commit)
            self.assertGreater(replace.index(live_assignment), commit)
        self.assertNotIn("m_PartObjects", replace)

    def test_ghost_catalog_is_body_weapon_only(self) -> None:
        ghost = next(
            boss
            for boss in self.boss_catalog["bosses"]
            if boss["archetypeId"] == "BOSS_VALTAN_GHOST"
        )
        self.assertEqual([], ghost["armorModels"])
        self.assertEqual([], ghost["armorParts"])
        self.assertTrue(ghost["bodyModel"].startswith("Character/Valtan/Ghost/"))
        self.assertEqual("Character/Valtan/ValtanWeapon.wmodel", ghost["weaponModel"])


if __name__ == "__main__":
    unittest.main()
