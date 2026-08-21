#!/usr/bin/env python3

from __future__ import annotations

import hashlib
import importlib.util
import json
import sys
import unittest
from pathlib import Path


SCRIPT_PATH = Path(__file__).resolve()
REPOSITORY_ROOT = SCRIPT_PATH.parent.parent.parent


def load_module(name: str, path: Path):
    specification = importlib.util.spec_from_file_location(name, path)
    if specification is None or specification.loader is None:
        raise RuntimeError(f"could not load {path}")
    module = importlib.util.module_from_spec(specification)
    sys.modules[name] = module
    specification.loader.exec_module(module)
    return module


builder = load_module(
    "build_valtan_whirlwind_effect_canary_for_model_view",
    SCRIPT_PATH.parent / "build_valtan_whirlwind_effect_canary.py",
)


def raw_sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


class ValtanModelViewCompositionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.resource_root = REPOSITORY_ROOT / "Client/Bin/Resources"
        cls.body_path = (
            cls.resource_root / "Character/Valtan/MN_RPBF_01.wmodel"
        )
        cls.weapon_path = (
            cls.resource_root / "Character/Valtan/ValtanWeapon.wmodel"
        )
        cls.animset_path = (
            cls.resource_root
            / "Character/Valtan/AnimSets/MN_RPBF_01_AnimSet.wmodel"
        )

    def test_boss_catalog_joins_exact_product_bundle(self) -> None:
        catalog = json.loads(
            (REPOSITORY_ROOT / "Data/Actors/BossCatalog.json").read_text(
                encoding="utf-8-sig"
            )
        )
        self.assertEqual(catalog["schema"], "lostark.boss-catalog")
        self.assertEqual(catalog["formatVersion"], 3)
        self.assertEqual(len(catalog["bosses"]), 1)
        valtan = catalog["bosses"][0]
        self.assertEqual(valtan["archetypeId"], "BOSS_VALTAN")
        self.assertEqual(
            valtan["bodyModel"], "Character/Valtan/MN_RPBF_01.wmodel"
        )
        self.assertEqual(
            valtan["weaponModel"], "Character/Valtan/ValtanWeapon.wmodel"
        )
        self.assertEqual(
            valtan["animationSetId"],
            "Character/Valtan/AnimSets/MN_RPBF_01_AnimSet.wmodel",
        )
        self.assertEqual(
            valtan["clientPresentationId"], "boss.valtan.client.v1"
        )
        self.assertEqual(valtan["presentationStatus"], "complete")
        self.assertEqual(
            valtan["combatObjectVisuals"],
            [
                {
                    "combatObjectArchetypeId":
                        "combatobject.valtan.high-jump.target-axe",
                    "clientVisualId":
                        "combatobject.visual.valtan.high-jump.target-axe.v1",
                    "effectAssetId": "effect.valtan.sky-axe.active",
                },
                {
                    "combatObjectArchetypeId":
                        "combatobject.valtan.red-blade-wave.projectile",
                    "clientVisualId":
                        "combatobject.visual.valtan.red-blade-wave.projectile.v1",
                    "effectAssetId": "effect.valtan.red-blade-wave.active",
                },
            ],
        )

    def test_product_bundle_bytes_are_pinned(self) -> None:
        expected = {
            self.body_path: (
                11_245_828,
                "227191781b035b9aa41d60cc8c49bf1e8b2f67a749111e4afe8f6f3c997b2011",
            ),
            self.weapon_path: (
                139_664,
                "baffbd5268f216267d1cba8fb9eaf2b58276122ddd7755d18787f7dd9bb9d3da",
            ),
            self.animset_path: (
                46_540_308,
                "4fd71652bf4ca6b11607449b7ece83509d9b3886b1bcdde9e2dc49c5b8c561c0",
            ),
        }
        for path, (size, digest) in expected.items():
            with self.subTest(path=path):
                self.assertTrue(path.is_file())
                self.assertEqual(path.stat().st_size, size)
                self.assertEqual(raw_sha256(path), digest)

    def test_body_owns_one_exact_weapon_socket(self) -> None:
        bones = builder.read_wmodel_bones(self.body_path)
        sockets = [bone for bone in bones if bone.name.casefold() == "b_wp_r_01"]
        self.assertEqual(len(bones), 87)
        self.assertEqual(len(sockets), 1)
        socket = sockets[0]
        self.assertEqual(socket.index, 54)
        self.assertEqual(socket.parent_index, 41)
        self.assertEqual(socket.name_hash, 0x51BF337724F3B4CA)

    def test_static_weapon_has_no_skeleton_contract(self) -> None:
        with self.assertRaisesRegex(
            builder.ContractError,
            "exactly one physical skeleton section",
        ):
            builder.read_wmodel_bones(self.weapon_path)

    def test_model_view_stages_product_valtan_and_skips_body_only_boss(self) -> None:
        loader = (REPOSITORY_ROOT / "Client/Private/Loader.cpp").read_text(
            encoding="utf-8-sig"
        )
        panel = (
            REPOSITORY_ROOT / "Client/Private/CharacterPreviewPanel.cpp"
        ).read_text(encoding="utf-8-sig")
        valtan_header = (
            REPOSITORY_ROOT / "Client/Public/Valtan.h"
        ).read_text(encoding="utf-8-sig")
        self.assertIn(
            "CValtanPresentationAssetService::Ensure_Prototypes", loader
        )
        self.assertIn(
            "HRESULT CLoader::Ready_ValtanPresentation", loader
        )
        self.assertIn(
            "return CValtanPresentationAssetService::Ensure_Prototypes(", loader
        )
        self.assertIn('TEXT("Prototype_GameObject_Valtan")', panel)
        self.assertIn("CValtan::MODEL_VIEW_SCALE", panel)
        self.assertIn("stagedValtan", panel)
        self.assertIn('WEAPON_PART_TAG = TEXT("Part_Weapon_R")', valtan_header)
        self.assertIn('WEAPON_SOCKET_BONE = "b_wp_r_01"', valtan_header)

    def test_visual_root_and_transactional_preview_contract_are_present(self) -> None:
        valtan = (REPOSITORY_ROOT / "Client/Private/Valtan.cpp").read_text(
            encoding="utf-8-sig"
        )
        target = (
            REPOSITORY_ROOT / "Client/Private/AnimationTargetService.cpp"
        ).read_text(encoding="utf-8-sig")
        presentation = (
            REPOSITORY_ROOT / "Client/Private/Effect_PresentationService.cpp"
        ).read_text(encoding="utf-8-sig")
        panel = (
            REPOSITORY_ROOT / "Client/Private/CharacterPreviewPanel.cpp"
        ).read_text(encoding="utf-8-sig")
        self.assertIn(
            "m_pBodyVisualRootCom->Get_WorldMatrixPtr()) *", valtan
        )
        self.assertIn(
            "m_pTransformCom->Get_WorldMatrixPtr()))", valtan
        )
        self.assertIn("s_PreviewBoss", target)
        self.assertIn("Try_Get_PresentationRootMatrix(pOut)", target)
        self.assertIn("Try_Get_PresentationRoot(PresentationRoot)", presentation)
        self.assertIn("stagedParentMatrixIndex", panel)
        self.assertLess(panel.index("Release(true);"), panel.index(
            "m_iPreviewParentMatrixIndex = stagedParentMatrixIndex;"
        ))


if __name__ == "__main__":
    unittest.main()
