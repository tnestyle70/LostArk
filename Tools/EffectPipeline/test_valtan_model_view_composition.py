#!/usr/bin/env python3

from __future__ import annotations

import hashlib
import importlib.util
import json
import math
import struct
import sys
import unittest
from pathlib import Path


SCRIPT_PATH = Path(__file__).resolve()
REPOSITORY_ROOT = SCRIPT_PATH.parent.parent.parent
LEVEL_PLACEMENT_EXTRACTOR = (
    REPOSITORY_ROOT / "Tools/LevelPlacementExtractor"
)


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
if str(LEVEL_PLACEMENT_EXTRACTOR) not in sys.path:
    sys.path.insert(0, str(LEVEL_PLACEMENT_EXTRACTOR))
floor_contract = load_module(
    "test_valtan_floor_emissive_contract_for_model_view",
    LEVEL_PLACEMENT_EXTRACTOR / "test_valtan_floor_emissive_contract.py",
)


ghost_bake = load_module(
    "bake_ghost_valtan_animset_for_model_view",
    REPOSITORY_ROOT / "Tools/ModelAssetConverter/bake_ghost_valtan_animset.py",
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
        cls.ghost_body_path = (
            cls.resource_root / "Character/Valtan/Ghost/MN_RPBF_02.wmodel"
        )
        cls.ghost_animset_path = (
            cls.resource_root / "Character/Valtan/Ghost/MN_RPBF_02_AnimSet.wmodel"
        )
        cls.armor_part_paths = (
            cls.resource_root
            / "Character/Valtan/MN_RPBF_01_Parts1.wmodel",
            cls.resource_root
            / "Character/Valtan/MN_RPBF_01_Parts2.wmodel",
        )

    def test_boss_catalog_joins_exact_product_bundle(self) -> None:
        catalog = json.loads(
            (REPOSITORY_ROOT / "Data/Actors/BossCatalog.json").read_text(
                encoding="utf-8-sig"
            )
        )
        self.assertEqual(catalog["schema"], "lostark.boss-catalog")
        self.assertEqual(catalog["formatVersion"], 7)
        self.assertEqual({row["archetypeId"] for row in catalog["bosses"]},
                         {
                             "BOSS_VALTAN",
                             "BOSS_VALTAN_GHOST",
                             "BOSS_KAKULSAYDON_G1_KOUKU",
                         })
        valtan = next(row for row in catalog["bosses"]
                      if row["archetypeId"] == "BOSS_VALTAN")
        self.assertEqual(valtan["archetypeId"], "BOSS_VALTAN")
        self.assertEqual(valtan["presentationScale"], 1.0)
        self.assertEqual(valtan["bodyModelPreScale"], 0.0001)
        self.assertEqual(valtan["weaponModelPreScale"], 100.0)
        self.assertEqual(
            valtan["bodyModel"], "Character/Valtan/MN_RPBF_01.wmodel"
        )
        self.assertEqual(
            valtan["weaponModel"], "Character/Valtan/ValtanWeapon.wmodel"
        )
        self.assertEqual(
            valtan["armorModels"],
            [
                "Character/Valtan/MN_RPBF_01_Parts1.wmodel",
                "Character/Valtan/MN_RPBF_01_Parts2.wmodel",
            ],
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
                        "combatobject.visual.valtan.high-jump.target-axe.v2",
                    "effectAssetId": "effect.valtan.sky-axe.active",
                    "effectV2Group": {
                        "groupId": "boss.valtan.axe",
                        "playbackRate": 47 / 24,
                        "visualHitMs": 2350,
                        "serverHitId": "hit.valtan.high-jump.target-axe.01",
                    },
                },
                {
                    "combatObjectArchetypeId":
                        "combatobject.valtan.red-blade-wave.projectile",
                    "clientVisualId":
                        "combatobject.visual.valtan.red-blade-wave.projectile.v1",
                    "effectAssetId": "effect.valtan.red-blade-wave.active",
                },
                {
                    "combatObjectArchetypeId":
                        "combatobject.valtan.fist-in-out.donut",
                    "clientVisualId":
                        "combatobject.visual.valtan.fist-in-out.donut.v1",
                    "effectAssetId":
                        "effect.valtan.carrier-v1.attack.fist-in-out.inner.clip-01",
                    "worldScale": [1.5, 1.5, 1.5],
                },
                {
                    "combatObjectArchetypeId":
                        "combatobject.valtan.fist-in-out.donut-large",
                    "clientVisualId":
                        "combatobject.visual.valtan.fist-in-out.donut-large.v1",
                    "effectAssetId": "effect.valtan.project-tuned.large-donut",
                    "worldScale": [1.5, 1.5, 1.5],
                },
                {
                    "combatObjectArchetypeId":
                        "combatobject.valtan.ground-roar.rock",
                    "clientVisualId":
                        "combatobject.visual.valtan.ground-roar.rock.v1",
                    "effectAssetId": "effect.valtan.ground-roar.rock.active",
                    "hitEffectAssetId":
                        "effect.valtan.ground-roar.rock.explode",
                },
                {
                    "combatObjectArchetypeId":
                        "combatobject.valtan.six-pizza.rock-pillar",
                    "clientVisualId":
                        "combatobject.visual.valtan.six-pizza.rock-pillar.v1",
                    "effectAssetId": "effect.valtan.six-pizza.rock.active",
                    "hitEffectAssetId":
                        "effect.valtan.six-pizza.rock.explode",
                },
                {
                    "combatObjectArchetypeId":
                        "combatobject.valtan.struggling.rock-pillar",
                    "clientVisualId":
                        "combatobject.visual.valtan.struggling.rock-pillar.v1",
                    "effectAssetId": "effect.valtan.struggling.rock.active",
                    "hitEffectAssetId":
                        "effect.valtan.struggling.rock.explode",
                },
                {
                    "combatObjectArchetypeId":
                        "combatobject.valtan.part-break.rock",
                    "clientVisualId":
                        "combatobject.visual.valtan.part-break.rock.v1",
                    "effectAssetId": "effect.valtan.ground-roar.rock.active",
                    "hitEffectAssetId":
                        "effect.valtan.ground-roar.rock.explode",
                },
                {
                    "combatObjectArchetypeId":
                        "combatobject.valtan.ghost.portal-charge",
                    "clientVisualId":
                        "combatobject.visual.valtan.ghost.portal-charge.v2",
                    "effectAssetId":
                        "effect.valtan.project-tuned.sequence.warp.portal",
                    "effectV2Group": {
                        "groupId": "boss.valtan.portal",
                        "playbackRate": 1.0,
                    },
                },
            ],
        )

    def test_ghost_catalog_joins_the_shared_product_presentation(self) -> None:
        catalog = json.loads((REPOSITORY_ROOT / "Data/Actors/BossCatalog.json")
                             .read_text(encoding="utf-8-sig"))
        normal = next(row for row in catalog["bosses"]
                      if row["archetypeId"] == "BOSS_VALTAN")
        ghost = next(row for row in catalog["bosses"]
                     if row["archetypeId"] == "BOSS_VALTAN_GHOST")
        self.assertEqual(ghost["bodyModel"],
                         "Character/Valtan/Ghost/MN_RPBF_02.wmodel")
        self.assertEqual(ghost["animationSetId"],
                         "Character/Valtan/Ghost/MN_RPBF_02_AnimSet.wmodel")
        self.assertEqual(ghost["weaponModel"], normal["weaponModel"])
        self.assertEqual(ghost["clientPresentationId"], normal["clientPresentationId"])
        self.assertEqual(ghost["serverProfileId"], "boss.valtan.ghost.server.v1")
        self.assertEqual(ghost["presentationClips"], normal["presentationClips"])
        self.assertEqual(ghost["bodyModelPreScale"], 0.01)
        self.assertEqual(ghost["weaponModelPreScale"], 1.0)
        self.assertEqual(ghost["presentationScale"], 1.0)
        self.assertEqual(ghost["armorModels"], [])
        self.assertEqual(ghost["armorParts"], [])
        self.assertEqual(ghost["combatObjectVisuals"], [])
        for key in ("bodyModel", "animationSetId", "weaponModel"):
            candidate = (self.resource_root / ghost[key]).resolve()
            self.assertTrue(candidate.is_relative_to(self.resource_root.resolve()))
            self.assertTrue(candidate.is_file(), ghost[key])

    def test_ghost_donor_matches_actual_skeleton_and_all_three_attack_clips(self) -> None:
        body_bytes = self.ghost_body_path.read_bytes()
        donor_bytes = self.ghost_animset_path.read_bytes()
        body_count, body_sections = ghost_bake.read_sections(body_bytes)
        donor_count, donor_sections = ghost_bake.read_sections(donor_bytes)
        body_bones = builder.read_wmodel_bones(self.ghost_body_path)
        donor_bones = builder.read_wmodel_bones(self.ghost_animset_path)
        self.assertEqual(len(body_bones), 87)
        self.assertEqual(body_bones, donor_bones,
                         "Attach_AnimationSet must bind the body's ordered bone identities")
        sockets = [bone for bone in body_bones if bone.name == "b_wp_r_01"]
        self.assertEqual([(bone.index, bone.parent_index) for bone in sockets], [(54, 41)])
        body_hash = ghost_bake.skeleton_hash(
            ghost_bake.skeleton_bones(body_bytes, body_sections))
        donor_hash = ghost_bake.skeleton_hash(
            ghost_bake.skeleton_bones(donor_bytes, donor_sections))
        self.assertNotEqual(body_hash, 0)
        self.assertEqual(body_hash, donor_hash)
        body_clips = {row["name"] for row in body_sections
                      if row["type"] == ghost_bake.SECTION_ANIMATION}
        donor_clips = {row["name"] for row in donor_sections
                       if row["type"] == ghost_bake.SECTION_ANIMATION}
        self.assertEqual((len(body_clips), len(donor_clips)), (body_count, donor_count))
        self.assertTrue(body_clips.isdisjoint(donor_clips),
                        "a duplicate clip rejects the atomic donor attach")
        presentation = json.loads((REPOSITORY_ROOT / "Data/Valtan/Valtan.presentation.json")
                                  .read_text(encoding="utf-8-sig"))
        attack_ids = {"VALTAN_WHIRLWIND", "VALTAN_FOUR_SLASH", "VALTAN_SEQUENCE_FOUR"}
        attacks = [row for row in presentation["patterns"]
                   if row["patternId"] in attack_ids]
        self.assertEqual({row["patternId"] for row in attacks}, attack_ids)
        required_clips = {occurrence["clip"] for attack in attacks
                          for stage in attack["stages"]
                          for occurrence in stage["animation"].get("occurrences", [])}
        self.assertTrue(required_clips)
        self.assertFalse(required_clips - (body_clips | donor_clips))
        for section in donor_sections:
            if section["type"] != ghost_bake.SECTION_ANIMATION:
                continue
            trailer_at = (ghost_bake.FILE_HEADER_SIZE + section["offset"] +
                          section["size"] - ghost_bake.ANIMATION_TRAILER_SIZE)
            self.assertEqual(struct.unpack_from("<Q", donor_bytes, trailer_at)[0],
                             body_hash, section["name"])

    def test_normal_and_ghost_death_clips_have_finite_actual_model_durations(self) -> None:
        catalog = json.loads((REPOSITORY_ROOT / "Data/Actors/BossCatalog.json")
                             .read_text(encoding="utf-8-sig"))
        for actor in catalog["bosses"]:
            if actor["archetypeId"] not in {"BOSS_VALTAN", "BOSS_VALTAN_GHOST"}:
                continue
            with self.subTest(archetype=actor["archetypeId"]):
                death_clips = []
                for asset in (actor["bodyModel"], actor["animationSetId"]):
                    payload = (self.resource_root / asset).read_bytes()
                    _, sections = ghost_bake.read_sections(payload)
                    for section in sections:
                        if (section["type"] == ghost_bake.SECTION_ANIMATION and
                                section["name"] == actor["presentationClips"]["dead"]):
                            header = struct.unpack_from("<4sIffIIB", payload, section["payload_at"])
                            self.assertEqual(header[0], b"WANM")
                            duration, ticks_per_second = header[2:4]
                            self.assertTrue(math.isfinite(duration) and duration > 0)
                            self.assertTrue(math.isfinite(ticks_per_second) and ticks_per_second > 0)
                            self.assertTrue(math.isfinite(duration / ticks_per_second))
                            death_clips.append((asset, duration / ticks_per_second))
                self.assertEqual(len(death_clips), 1,
                                 "the actual combined model must resolve exactly one catalog death clip")

    def test_ghost_donor_translation_keys_use_the_catalog_body_unit_ratio(self) -> None:
        catalog = json.loads((REPOSITORY_ROOT / "Data/Actors/BossCatalog.json")
                             .read_text(encoding="utf-8-sig"))
        actors = {row["archetypeId"]: row for row in catalog["bosses"]}
        ratio = (actors["BOSS_VALTAN"]["bodyModelPreScale"] /
                 actors["BOSS_VALTAN_GHOST"]["bodyModelPreScale"])
        source = self.animset_path.read_bytes()
        donor = self.ghost_animset_path.read_bytes()
        _, source_sections = ghost_bake.read_sections(source)
        _, donor_sections = ghost_bake.read_sections(donor)
        donor_clips = {row["name"]: row for row in donor_sections
                       if row["type"] == ghost_bake.SECTION_ANIMATION}
        sampled_keys = 0
        for source_section in source_sections:
            if source_section["type"] != ghost_bake.SECTION_ANIMATION:
                continue
            donor_section = donor_clips[source_section["name"]]
            source_base = source_section["payload_at"]
            donor_base = donor_section["payload_at"]
            source_channels = struct.unpack_from("<I", source, source_base + 4)[0]
            donor_channels = struct.unpack_from("<I", donor, donor_base + 4)[0]
            self.assertEqual(source_channels, donor_channels)
            source_table = source_base + ghost_bake.ANIMATION_META_SIZE
            donor_table = donor_base + ghost_bake.ANIMATION_META_SIZE
            source_keys = source_table + source_channels * ghost_bake.ANIMATION_CHANNEL_SIZE
            donor_keys = donor_table + donor_channels * ghost_bake.ANIMATION_CHANNEL_SIZE
            for channel in range(source_channels):
                source_count, source_offset = struct.unpack_from(
                    "<II", source, source_table + channel * ghost_bake.ANIMATION_CHANNEL_SIZE + 8)
                donor_count, donor_offset = struct.unpack_from(
                    "<II", donor, donor_table + channel * ghost_bake.ANIMATION_CHANNEL_SIZE + 8)
                self.assertEqual(source_count, donor_count)
                if source_count == 0:
                    continue
                for index in {0, source_count - 1}:
                    expected = struct.unpack_from("<4f", source,
                        source_keys + source_offset + index * ghost_bake.VECTOR_KEY_SIZE)
                    actual = struct.unpack_from("<4f", donor,
                        donor_keys + donor_offset + index * ghost_bake.VECTOR_KEY_SIZE)
                    self.assertEqual(expected[0], actual[0])
                    for reference, value in zip(expected[1:], actual[1:]):
                        self.assertTrue(math.isclose(value, reference * ratio,
                                                    rel_tol=1.0e-6, abs_tol=1.0e-6),
                                        source_section["name"])
                    sampled_keys += 1
        self.assertGreater(sampled_keys, 0)

    def test_ghost_material_dependencies_resolve_inside_resources(self) -> None:
        materials = floor_contract.parse_wmodel_materials(self.ghost_body_path)
        self.assertTrue(materials)
        expected = {
            "textures/mn_rpbf_01_ghost_d.dds",
            "textures/mn_rpbf_01_n.dds",
            "textures/mn_rpbf_01-1_d_loc_int.dds",
            "textures/mn_rpbf_01-1_n_loc_int.dds",
            "textures/mn_rpbf_01-2_d_loc_int.dds",
            "textures/mn_rpbf_01-2_n_loc_int.dds",
        }
        referenced = set()
        for slots in materials.values():
            self.assertTrue(slots["baseColor"])
            self.assertTrue(slots["normal"])
            for relative_path in slots.values():
                if not relative_path:
                    continue
                self.assertTrue(relative_path.startswith("textures/"))
                candidate = (self.ghost_body_path.parent / relative_path).resolve()
                self.assertTrue(candidate.is_relative_to(self.resource_root.resolve()))
                self.assertTrue(candidate.is_file(), relative_path)
                with candidate.open("rb") as texture:
                    self.assertEqual(texture.read(4), b"DDS ", relative_path)
                referenced.add(relative_path)
        self.assertEqual(referenced, expected)
        # The donor carries no render-texture dependency: CModel attaches only its clips.
        donor_materials = floor_contract.parse_wmodel_materials(self.ghost_animset_path)
        self.assertFalse({path for slots in donor_materials.values()
                          for path in slots.values() if path})

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
            self.armor_part_paths[0]: (
                178_344,
                "8db32a0097a7dcf9eabe1adfeac2288f68f133bf1739626fe2d9321146182f8b",
            ),
            self.armor_part_paths[1]: (
                233_544,
                "9096c312af174ca6836b0afdce6955afb39c2ecdfcf7a332a540e5755ada4f03",
            ),
        }
        for path, (size, digest) in expected.items():
            with self.subTest(path=path):
                self.assertTrue(path.is_file())
                self.assertEqual(path.stat().st_size, size)
                self.assertEqual(raw_sha256(path), digest)

    def test_product_bundle_material_dependency_closure_is_pinned(self) -> None:
        expected = {
            "Character/Valtan/textures/mn_rpbf_01-1_d_loc_int.tga": (
                2_077_007,
                "08f7c503120b490b65930c26443ed2a93200bf76f242ea9d7f9fa19c6b2861f8",
            ),
            "Character/Valtan/textures/mn_rpbf_01-1_e_loc_int.tga": (
                80_729,
                "9e2a0aa576a07c7fcc0e160b699e019c2ca396362630ac3d812a8762eb073fe0",
            ),
            "Character/Valtan/textures/mn_rpbf_01-1_n_loc_int.tga": (
                1_803_671,
                "10c163a36731cacaa7bbeca50406e48496ab0580a758afce60b074ce8ed4119c",
            ),
            "Character/Valtan/textures/mn_rpbf_01-1_s_loc_int.tga": (
                899_431,
                "c8369c517579e2cae57b0c4286f65b8eb9c8fd087e7ef6049d507a6e56ccdadd",
            ),
            "Character/Valtan/textures/mn_rpbf_01-2_d_loc_int.tga": (
                1_937_817,
                "c07e06c57145e089739eaf3176f171300ba95e36d9cf85cd56ea001121acdb1e",
            ),
            "Character/Valtan/textures/mn_rpbf_01-2_e.tga": (
                118_645,
                "f166d6ef9932faa43ae92e854204647e007ed9738d3e23fa6e82feb5ed978614",
            ),
            "Character/Valtan/textures/mn_rpbf_01-2_n_loc_int.tga": (
                3_034_736,
                "39e977c07f4d511fabf2faee3cf67e7464f459e07c136dd3b86c8a30541c2fc5",
            ),
            "Character/Valtan/textures/mn_rpbf_01-2_s_loc_int.tga": (
                857_089,
                "26a900fb82b256dc99eed1b1369471713e59b3fabb8d3fe891fecdd71d31af61",
            ),
            "Character/Valtan/textures/mn_rpbf_01_d.tga": (
                2_983_165,
                "03364ba28d897c4ba14ada4af4cce90e1e78879c7b23587b6479767631d84d8c",
            ),
            "Character/Valtan/textures/mn_rpbf_01_e.tga": (
                168_105,
                "70ff1d7b25f1c7246d4dd5fd1f3852d286b3174afbb8c5795aa0257808ef0de2",
            ),
            "Character/Valtan/textures/mn_rpbf_01_n.tga": (
                2_305_531,
                "52ab0301d3c7e4d89ae6793be9a2001be4e966dd7137d04f731a36bd8f754b5a",
            ),
            "Character/Valtan/textures/mn_rpbf_01_s.tga": (
                883_529,
                "911e6aefe61b314072d8152e4c2298fc059cd59804edb3ad1d311e3c5d91e466",
            ),
            "Character/Valtan/textures/wp_mn_rpbf_01-1_d.dds": (
                131_200,
                "d96edae6b1abf000a72213e177f3ddac5bebafa935991702b34141c397a2936d",
            ),
            "Character/Valtan/textures/wp_mn_rpbf_01-1_e.dds": (
                131_200,
                "b1e51c9333ec78ab078a49abb147598f45dc3e11dfb481b385bd10507ce9e3b1",
            ),
            "Character/Valtan/textures/wp_mn_rpbf_01-1_n.dds": (
                262_272,
                "c9f7e89c33d87b8250a07d5a2e7463731b1e413f648bda5e6cbf58a8a23a4d3d",
            ),
            "Character/Valtan/textures/wp_mn_rpbf_01-1_s.dds": (
                262_272,
                "8c671844df1bdc3df8ef327e282639b88926295491e80e18affa1cabaf147945",
            ),
            "Character/Valtan/textures/wp_mn_rpbf_01_d.dds": (
                131_200,
                "0c43ce060328be3a52e92bfc71aaaf724c6dca60a8e105e00ecdc2fff1a3ca15",
            ),
            "Character/Valtan/textures/wp_mn_rpbf_01_e.dds": (
                131_200,
                "2a1f6b86415ca99ab91952ba29031f1aa22c91052cdb1155296e8224404cea38",
            ),
            "Character/Valtan/textures/wp_mn_rpbf_01_n.dds": (
                262_272,
                "e287ce49b1620ffaa62e16d89a9920036c25a5ee455b13ae0009b776f8a94c29",
            ),
            "Character/Valtan/textures/wp_mn_rpbf_01_s.dds": (
                65_664,
                "f6908170a8d0d94c95655739fa609a9493fd9c9653305cfa82f4ceb2aff97c19",
            ),
        }

        material_asset_ids: set[str] = set()
        for model_path in (
            self.body_path,
            *self.armor_part_paths,
            self.weapon_path,
        ):
            materials = floor_contract.parse_wmodel_materials(model_path)
            for slots in materials.values():
                for relative_path in slots.values():
                    if relative_path:
                        self.assertTrue(relative_path.startswith("textures/"))
                        material_asset_ids.add(
                            f"Character/Valtan/{relative_path}"
                        )

        self.assertEqual(material_asset_ids, set(expected))
        for asset_id, (size, digest) in expected.items():
            with self.subTest(asset_id=asset_id):
                path = self.resource_root / asset_id
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

    def test_body_radius_fits_product_locomotion_geometry(self) -> None:
        geometry = load_module(
            "valtan_body_radius_geometry",
            REPOSITORY_ROOT
            / "Tools/ModelAssetConverter/verify_dimensionmaster_summon_bind_pose.py",
        )
        body = geometry.read_wmodel(self.body_path)
        animset = geometry.read_wmodel(self.animset_path)
        catalog = json.loads(
            (REPOSITORY_ROOT / "Data/Actors/BossCatalog.json")
            .read_text(encoding="utf-8-sig")
        )
        profiles = json.loads(
            (REPOSITORY_ROOT / "Data/Balance/BossProfiles.json")
            .read_text(encoding="utf-8-sig")
        )
        actor = next(row for row in catalog["bosses"]
                     if row["archetypeId"] == "BOSS_VALTAN")
        profile = next(row for row in profiles["bosses"]
                       if row["archetypeId"] == actor["archetypeId"])
        # CValtanPresentationAssetService's WModel pretransform, followed by
        # the catalog visual scale. Body yaw preserves this XZ radius.
        world_scale = actor["bodyModelPreScale"] * actor["presentationScale"]
        bone_indices = {bone.name: index
                        for index, bone in enumerate(body.skeleton_bones)}
        root_index = bone_indices["b_root"]
        locomotion = {"mesh_idle_battle_1", "mesh_run_battle_1"}
        clips = [clip for clip in animset.animations if clip.name in locomotion]
        self.assertEqual({clip.name for clip in clips}, locomotion)
        support_vertices = []
        for vertex in body.vertices:
            dominant = max(range(4), key=lambda index: vertex.weights[index])
            bone_name = body.mesh_bones[vertex.indices[dominant]].name
            # Arms, weapon and horns belong to authored attack shapes, not
            # the body movement cylinder. Keep the torso and every leg/foot.
            if any(part in bone_name for part in
                   ("pelvis", "spine", "thigh", "calf", "foot", "toe")):
                support_vertices.append(vertex)
        self.assertTrue(support_vertices)
        maximum_radius = 0.0
        for clip in clips:
            times = sorted({float(tick)
                            for tick in range(math.floor(clip.duration_ticks) + 1)}
                           | {clip.duration_ticks})
            for time in times:
                local = [list(bone.transform) for bone in body.skeleton_bones]
                for channel in clip.channels:
                    index = bone_indices[
                        animset.skeleton_bones[channel.bone_index].name]
                    local[index] = geometry.affine_matrix(
                        geometry.sample_vector(channel.scale_keys, time, (1, 1, 1)),
                        geometry.sample_quaternion(channel.rotation_keys, time),
                        geometry.sample_vector(channel.position_keys, time, (0, 0, 0)),
                    )
                # Product CValtan suppresses all three b_root translations;
                # server motion must not be counted as a larger body radius.
                local[root_index][12:15] = (
                    body.skeleton_bones[root_index].transform[12:15])
                combined = geometry.combined_transforms(body.skeleton_bones, local)
                skin = [geometry.matrix_multiply(bone.transform, combined[index])
                        for index, bone in enumerate(body.mesh_bones)]
                for vertex in support_vertices:
                    weight_sum = sum(vertex.weights)
                    self.assertGreater(weight_sum, 0.0)
                    point = [0.0, 0.0, 0.0]
                    for index, weight in zip(vertex.indices, vertex.weights):
                        if weight <= 0.0:
                            continue
                        posed = geometry.transform_point(vertex.position, skin[index])
                        for axis in range(3):
                            point[axis] += posed[axis] * weight / weight_sum * world_scale
                    maximum_radius = max(maximum_radius, math.hypot(point[0], point[2]))
        radius = profile["collisionRadius"]
        self.assertEqual(radius, 1.4)
        self.assertGreaterEqual(radius, maximum_radius,
                                "body collision must cover the locomotion footprint")
        self.assertLessEqual(radius, maximum_radius + 0.1 * actor["presentationScale"],
                             "body collision must not retain the old oversized radius")

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
        replication = (
            REPOSITORY_ROOT / "Client/Private/ClientReplication.cpp"
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
        self.assertIn("desc.fScale = pBoss->presentationScale;", panel)
        self.assertIn("desc.fScale = pBoss->presentationScale;", replication)
        self.assertNotIn("MODEL_VIEW_SCALE", valtan_header)
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

    def test_pattern_effect_scale_policy_preserves_world_footprints(self) -> None:
        service = (
            REPOSITORY_ROOT / "Client/Private/Effect_PresentationService.cpp"
        ).read_text(encoding="utf-8-sig")
        service_header = (
            REPOSITORY_ROOT / "Client/Public/Effect_PresentationService.h"
        ).read_text(encoding="utf-8-sig")
        effect_tool = (
            REPOSITORY_ROOT / "Client/Private/Effect_Tool.cpp"
        ).read_text(encoding="utf-8-sig")
        valtan = (REPOSITORY_ROOT / "Client/Private/Valtan.cpp").read_text(
            encoding="utf-8-sig"
        )
        tree = (
            REPOSITORY_ROOT / "Client/Private/ValtanPatternTree.cpp"
        ).read_text(encoding="utf-8-sig")
        self.assertIn("Try_BuildCueScalePolicyAnchor", service)
        self.assertIn("WorldScale.x / fScaleX", service)
        self.assertIn("Build_CueScalePolicyAnchor", service_header)
        self.assertIn("Build_CueScalePolicyRoot", service_header)
        self.assertIn(
            "Owner, Effect.eScalePolicy, Effect.vWorldScale", service
        )
        self.assertGreaterEqual(
            effect_tool.count(
                "CEffectPresentationService::Build_CueScalePolicyRoot("
            ),
            2,
        )
        self.assertIn(
            "CEffectPresentationService::Build_CueScalePolicyAnchor(",
            effect_tool,
        )

        helper_start = effect_tool.index(
            "bool Build_ToolValtanSourceAnchorWorld("
        )
        live_start = effect_tool.index(
            "bool Resolve_ToolSourceAnchorWorlds("
        )
        helper_scope = effect_tool[helper_start:live_start]
        live_end = effect_tool.index(
            "bool Is_CompilerOwnedPortableRecipe(", live_start
        )
        live_scope = effect_tool[live_start:live_end]
        history_start = effect_tool.index(
            "bool_t Client::CEffect_Tool::Seek_WorldPreviewWithSourceAnchorHistory("
        )
        history_end = effect_tool.index(
            "bool_t Client::CEffect_Tool::Is_ProductCueVisible(",
            history_start,
        )
        history_scope = effect_tool[history_start:history_end]
        self.assertIn("Build_CueScalePolicyAnchor(", helper_scope)
        self.assertIn("AnchorBuild.RawBone = RawBone;", helper_scope)
        self.assertIn(
            "AnchorBuild.OwnerWorld = EffectiveOwnerRoot;", helper_scope
        )
        self.assertIn("Build_SourceBoneAnchorWorld(", helper_scope)
        self.assertIn(
            "const Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW* pValtanCue",
            live_scope,
        )
        self.assertIn("pValtanModel->Get_BoneMatrix(", live_scope)
        self.assertIn("Build_ToolValtanSourceAnchorWorld(", live_scope)
        self.assertIn(
            "&m_ValtanProductPreview->Cue : nullptr", effect_tool
        )
        self.assertIn("PoseSample.BoneCombinedMatrices[iRequest]", history_scope)
        self.assertIn(
            "Build_ToolValtanSourceAnchorWorld(", history_scope
        )
        self.assertIn(
            "Source-anchor history could not normalize the Valtan source bone",
            history_scope,
        )
        self.assertIn("AnchorBuild.OwnerWorld = EffectiveOwnerRoot;", effect_tool)
        self.assertIn("Staged.RootWorld = CueRoot;", effect_tool)
        self.assertIn("Desc.eScalePolicy = Cue.eScalePolicy;", valtan)
        self.assertIn("Desc.vWorldScale = Cue.vWorldScale;", valtan)
        self.assertIn(
            '"split presentation cue worldScale must preserve 1.5"', tree
        )
        self.assertIn(
            '"effect.valtan.pattern.420633.active.v1.unified"', effect_tool
        )
        self.assertIn(
            "Is_ValtanExactHistoryPreviewEffectAssetId(", effect_tool
        )
        self.assertIn("Matches_ValtanExactHistoryBinding(", effect_tool)
        self.assertGreaterEqual(
            effect_tool.count("Seek_ValtanBossPatternTransformHistory("), 8
        )
        prepare_start = effect_tool.index(
            "bool_t Client::CEffect_Tool::Prepare_ValtanBossPatternTransformHistory("
        )
        prepare_end = effect_tool.index(
            "bool_t Client::CEffect_Tool::Build_ValtanBossPatternTransformSample(",
            prepare_start,
        )
        prepare_scope = effect_tool[prepare_start:prepare_end]
        self.assertIn("Matches_ValtanExactHistoryBinding(", prepare_scope)
        self.assertIn("5u : 3u;", prepare_scope)

        stage_start = effect_tool.index(
            "bool_t Client::CEffect_Tool::Stage_WorldPreview(\n"
            "\tconst EFFECT_DOCUMENT_DESC& Document,\n"
            "\tconst bool_t bAllowReadOnlySourceProjection)"
        )
        stage_end = effect_tool.index(
            "Client::CEffect_Tool::Build_PreviewDocument(", stage_start
        )
        particle_start = effect_tool.index(
            "bool_t Client::CEffect_Tool::Try_AuditionParticleSystem("
        )
        selected_start = effect_tool.index(
            "bool_t Client::CEffect_Tool::Try_AuditionSelectedElement("
        )
        selected_end = effect_tool.index(
            "bool_t Client::CEffect_Tool::Refresh_ResourceCatalog(",
            selected_start,
        )
        for scope in (
            effect_tool[stage_start:stage_end],
            effect_tool[particle_start:selected_start],
            effect_tool[selected_start:selected_end],
        ):
            self.assertIn("Seek_ValtanBossPatternTransformHistory(", scope)

        alias = json.loads(
            (
                REPOSITORY_ROOT
                / "Data/Effects/Authored/effect.valtan.pattern.420633.active.v1.unified.effect.json"
            ).read_text(encoding="utf-8-sig")
        )
        self.assertEqual(
            alias["effectAssetId"],
            "effect.valtan.pattern.420633.active.v1.unified",
        )
        visible_follow_carriers = [
            element["actionCueAttachment"]
            for element in alias["elements"]
            if element.get("visible")
            and element.get("material", {})
            .get("execution", {})
            .get("enabled")
            and element.get("actionCueAttachment", {}).get("enabled")
            and element["actionCueAttachment"].get("follow")
        ]
        self.assertEqual(len(visible_follow_carriers), 5)
        self.assertEqual(
            {
                (
                    attachment["sourceAnchorSlotId"],
                    attachment["runtimeAnchorSlotId"],
                    attachment["runtimeBoneName"],
                )
                for attachment in visible_follow_carriers
            },
            {("B_EffectRoot", "B_EffectRoot", "b_effectroot")},
        )
        self.assertTrue(
            all(
                attachment.get("follow")
                for attachment in visible_follow_carriers
            )
        )
        alias_pairs = json.loads(
            (
                REPOSITORY_ROOT
                / "Data/Animation/Authored/Valtan/Valtan.patterneffectv1aliases.json"
            ).read_text(encoding="utf-8-sig")
        )["aliases"]
        self.assertIn(
            {
                "effectAssetId": "effect.valtan.pattern.420633.active",
                "v1EffectAssetId": (
                    "effect.valtan.pattern.420633.active.v1.unified"
                ),
            },
            alias_pairs,
        )

    def test_gameplay_footprint_accepts_live_owner_scale_drift(self) -> None:
        service = (
            REPOSITORY_ROOT / "Client/Private/Effect_PresentationService.cpp"
        ).read_text(encoding="utf-8-sig")
        helper_start = service.index("bool_t Try_BuildCueScalePolicyAnchor(")
        helper_end = service.index(
            "bool_t Try_ComposeCueScalePolicyRoot(", helper_start
        )
        helper_scope = service[helper_start:helper_end]

        self.assertNotIn("CUE_OWNER_UNIFORM_SCALE_TOLERANCE", service)
        drift_start = service.index("constexpr bool_t Is_CueOwnerScaleDriftWithinLimit(")
        drift_end = service.index("static_assert(", drift_start)
        self.assertIn("CUE_OWNER_MAXIMUM_SCALE_DRIFT_RATIO",
                      service[drift_start:drift_end])
        self.assertIn("Is_CueOwnerScaleDriftWithinLimit(", service)
        self.assertIn("static_assert(Is_CueOwnerScaleDriftWithinLimit(", service)
        self.assertIn("static_assert(!Is_CueOwnerScaleDriftWithinLimit(", service)
        self.assertIn(
            "!Is_CueOwnerScaleDriftWithinLimit(", helper_scope
        )
        self.assertIn("Is_NonDegenerateAffineMatrix(SampledOwnerAnchor)", helper_scope)
        self.assertIn("fXY > CUE_OWNER_ORTHOGONAL_TOLERANCE", helper_scope)
        self.assertIn("fXZ > CUE_OWNER_ORTHOGONAL_TOLERANCE", helper_scope)
        self.assertIn("fYZ > CUE_OWNER_ORTHOGONAL_TOLERANCE", helper_scope)
        self.assertIn("fSignedNormalizedDeterminant <", helper_scope)
        self.assertIn("WorldScale.x / fScaleX", helper_scope)
        self.assertIn("WorldScale.y / fScaleY", helper_scope)
        self.assertIn("WorldScale.z / fScaleZ", helper_scope)

        def build_footprint_basis(rows):
            lengths = tuple(math.sqrt(sum(value * value for value in row))
                            for row in rows)
            if not all(math.isfinite(value) and value > 1.0e-8
                       for value in lengths):
                return None
            if ((max(lengths) - min(lengths)) > max(lengths) * 0.005):
                return None

            normalized_dots = (
                abs(sum(rows[0][i] * rows[1][i] for i in range(3)) /
                    (lengths[0] * lengths[1])),
                abs(sum(rows[0][i] * rows[2][i] for i in range(3)) /
                    (lengths[0] * lengths[2])),
                abs(sum(rows[1][i] * rows[2][i] for i in range(3)) /
                    (lengths[1] * lengths[2])),
            )
            determinant = (
                rows[0][0] *
                (rows[1][1] * rows[2][2] - rows[1][2] * rows[2][1]) -
                rows[0][1] *
                (rows[1][0] * rows[2][2] - rows[1][2] * rows[2][0]) +
                rows[0][2] *
                (rows[1][0] * rows[2][1] - rows[1][1] * rows[2][0])
            )
            signed_normalized_determinant = determinant / math.prod(lengths)
            if (not all(value <= 0.001 for value in normalized_dots) or
                    not math.isfinite(signed_normalized_determinant) or
                    signed_normalized_determinant < 0.999):
                return None

            world_scale = (1.5, 1.5, 1.5)
            return tuple(
                tuple(component * world_scale[axis] / lengths[axis]
                      for component in row)
                for axis, row in enumerate(rows)
            )

        live_drift = (
            (0.0, 0.0, 0.997025),
            (0.0, 1.0, 0.0),
            (-0.997025, 0.0, 0.0),
        )
        self.assertEqual(
            tuple(math.sqrt(sum(value * value for value in row))
                  for row in live_drift),
            (0.997025, 1.0, 0.997025),
        )
        normalized = build_footprint_basis(live_drift)
        self.assertIsNotNone(normalized)
        self.assertEqual(
            tuple(round(math.sqrt(sum(value * value for value in row)), 6)
                  for row in normalized),
            (1.5, 1.5, 1.5),
        )

        self.assertIsNone(build_footprint_basis(
            ((1.0, 0.0, 0.0), (0.02, 1.0, 0.0), (0.0, 0.0, 1.0))
        ))
        self.assertIsNone(build_footprint_basis(
            ((-1.0, 0.0, 0.0), (0.0, 1.0, 0.0), (0.0, 0.0, 1.0))
        ))
        self.assertIsNone(build_footprint_basis(
            ((0.0, 0.0, 0.0), (0.0, 1.0, 0.0), (0.0, 0.0, 1.0))
        ))
        self.assertIsNone(build_footprint_basis(
            ((math.nan, 0.0, 0.0), (0.0, 1.0, 0.0), (0.0, 0.0, 1.0))
        ))
        self.assertIsNone(build_footprint_basis(
            ((0.99, 0.0, 0.0), (0.0, 1.0, 0.0), (0.0, 0.0, 1.0))
        ))
        self.assertIsNone(build_footprint_basis(
            ((1.0e-7, 0.0, 0.0), (0.0, 1000.0, 0.0), (0.0, 0.0, 1.0))
        ))

    def test_high_jump_recovery_uses_a_non_loop_idle_hold(self) -> None:
        presentation = json.loads(
            (REPOSITORY_ROOT / "Data/Valtan/Valtan.presentation.json")
            .read_text(encoding="utf-8-sig")
        )
        high_jump = next(
            row for row in presentation["patterns"]
            if row["patternId"] == "VALTAN_HIGH_JUMP"
        )
        recovery = next(
            row for row in high_jump["stages"]
            if row["stageId"] == "RECOVERY"
        )
        occurrences = recovery["animation"]["occurrences"]
        self.assertEqual(len(occurrences), 1)
        self.assertEqual(occurrences[0]["clip"], "mesh_idle_battle_1")
        self.assertEqual(occurrences[0]["playMs"], 400)
        self.assertFalse(occurrences[0]["repeatUntilStageEnd"])


if __name__ == "__main__":
    unittest.main()
