#!/usr/bin/env python3

from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path

from build_skill_effect_resource_index import build_resource_index, write_resource_index


class SkillEffectResourceIndexTests(unittest.TestCase):
    def write_contract(
        self, root: Path, *, effect_id: str = "effect.test.skill.10"
    ) -> tuple[Path, Path, Path, Path]:
        player_skills = root / "Data/Balance/PlayerSkills.json"
        skill_bindings = root / "Data/Animation/Authored/Test/Test.skillbindings.json"
        authored_root = root / "Data/Effects/Authored"
        resource_root = root / "Client/Bin/Resources"
        player_skills.parent.mkdir(parents=True)
        skill_bindings.parent.mkdir(parents=True)
        authored_root.mkdir(parents=True)
        player_skills.write_text(
            json.dumps(
                {
                    "skills": [
                        {
                            "skillId": 10,
                            "characterClass": "TEST",
                            "inputSlot": "Q",
                            "displayName": "Test Q",
                            "actionId": "test.skill.10",
                            "effectId": effect_id,
                        }
                    ]
                }
            ),
            encoding="utf-8",
        )
        skill_bindings.write_text(
            json.dumps(
                {
                    "animationAssetId": "Test",
                    "characterClass": "TEST",
                    "bindings": [{"skillId": 10, "clips": ["clip_q"]}],
                }
            ),
            encoding="utf-8",
        )
        return player_skills, skill_bindings, authored_root, resource_root

    def test_collects_unique_runtime_assets_and_material_identities(self) -> None:
        with tempfile.TemporaryDirectory() as raw_root:
            root = Path(raw_root)
            player_skills, skill_bindings, authored_root, resource_root = (
                self.write_contract(root)
            )
            effect_id = "effect.test.skill.10"
            (authored_root / f"{effect_id}.effect.json").write_text(
                json.dumps(
                    {
                        "effectAssetId": effect_id,
                        "modelCues": [
                            {
                                "cueId": "summon",
                                "modelAssetId": "Character/Test/Summon.wmodel",
                            }
                        ],
                        "elements": [
                            {
                                "id": "emitter.1",
                                "resources": [
                                    {
                                        "slotId": "meshModel",
                                        "assetId": "Effect/Test/Meshes/A.wmodel",
                                    },
                                    {
                                        "slotId": "Base",
                                        "assetId": "Effect/Test/Textures/A.dds",
                                    },
                                ],
                                "material": {
                                    "templateId": "effect.source_material",
                                    "sourceMaterialPath": "FX_M.Mat_A",
                                    "sourceProfile": {
                                        "profileId": "profile.a",
                                        "parentMaterialPath": "FX_M.Parent_A",
                                        "runtimeShaderProfileId": "effect.ue3.test.v1",
                                        "semanticStatus": "reconstructed_profile",
                                    },
                                },
                            },
                            {
                                "id": "emitter.2",
                                "resources": [
                                    {
                                        "slotId": "Base",
                                        "assetId": "Effect/Test/Textures/A.dds",
                                    }
                                ],
                                "material": {
                                    "templateId": "effect.source_material",
                                    "sourceMaterialPath": "FX_M.Mat_A",
                                    "sourceProfile": {
                                        "profileId": "profile.a",
                                        "parentMaterialPath": "FX_M.Parent_A",
                                        "runtimeShaderProfileId": "effect.ue3.test.v1",
                                        "semanticStatus": "reconstructed_profile",
                                    },
                                },
                            },
                        ],
                    }
                ),
                encoding="utf-8",
            )
            for asset_id in (
                "Character/Test/Summon.wmodel",
                "Effect/Test/Meshes/A.wmodel",
                "Effect/Test/Textures/A.dds",
            ):
                path = resource_root / Path(asset_id)
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(b"asset")

            index, manifests = build_resource_index(
                player_skills,
                skill_bindings,
                authored_root,
                root,
                "TEST",
                ("Q",),
                resource_root,
            )

        manifest = manifests["Q"]
        self.assertEqual("INDEX_READY", manifest["bindingStatus"])
        self.assertEqual(["clip_q"], manifest["animationClips"])
        self.assertEqual(3, manifest["summary"]["assetCount"])
        self.assertEqual(1, manifest["summary"]["materialIdentityCount"])
        by_id = {row["assetId"]: row for row in manifest["assets"]}
        self.assertEqual(2, by_id["Effect/Test/Textures/A.dds"]["occurrenceCount"])
        self.assertEqual(["emitter.1", "emitter.2"], by_id["Effect/Test/Textures/A.dds"]["elementIds"])
        self.assertEqual("PRESENT", by_id["Character/Test/Summon.wmodel"]["physicalStatus"])
        self.assertEqual(3, index["summary"]["uniqueAssetCount"])

    def test_empty_effect_id_does_not_guess_an_authored_document(self) -> None:
        with tempfile.TemporaryDirectory() as raw_root:
            root = Path(raw_root)
            player_skills, skill_bindings, authored_root, _ = self.write_contract(
                root, effect_id=""
            )
            (authored_root / "effect.test.skill.10.effect.json").write_text(
                json.dumps(
                    {"effectAssetId": "effect.test.skill.10", "elements": []}
                ),
                encoding="utf-8",
            )

            _, manifests = build_resource_index(
                player_skills,
                skill_bindings,
                authored_root,
                root,
                "TEST",
                ("Q",),
            )

        self.assertEqual("MISSING_EFFECT_BINDING", manifests["Q"]["bindingStatus"])
        self.assertIsNone(manifests["Q"]["source"]["authoredEffect"])
        self.assertEqual([], manifests["Q"]["assets"])

    def test_missing_animation_binding_is_reported(self) -> None:
        with tempfile.TemporaryDirectory() as raw_root:
            root = Path(raw_root)
            player_skills, skill_bindings, authored_root, _ = self.write_contract(root)
            binding_document = json.loads(skill_bindings.read_text(encoding="utf-8"))
            binding_document["bindings"] = []
            skill_bindings.write_text(json.dumps(binding_document), encoding="utf-8")

            _, manifests = build_resource_index(
                player_skills,
                skill_bindings,
                authored_root,
                root,
                "TEST",
                ("Q",),
            )

        self.assertEqual("MISSING_ANIMATION_BINDING", manifests["Q"]["bindingStatus"])

    def test_writer_creates_only_json_reference_manifests(self) -> None:
        with tempfile.TemporaryDirectory() as raw_root:
            root = Path(raw_root)
            player_skills, skill_bindings, authored_root, _ = self.write_contract(
                root, effect_id=""
            )
            index, manifests = build_resource_index(
                player_skills,
                skill_bindings,
                authored_root,
                root,
                "TEST",
                ("Q",),
            )
            output_root = root / "Data/Effects/ResourceIndex/DimensionMaster"
            changed = write_resource_index(output_root, index, manifests)
            changed_again = write_resource_index(output_root, index, manifests)
            files = sorted(path for path in output_root.rglob("*") if path.is_file())

        self.assertEqual(2, changed)
        self.assertEqual(0, changed_again)
        self.assertEqual(2, len(files))
        self.assertTrue(all(path.suffix == ".json" for path in files))


if __name__ == "__main__":
    unittest.main()
