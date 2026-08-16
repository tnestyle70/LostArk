from __future__ import annotations

import copy
import importlib.util
import json
from pathlib import Path
import re
import sys
import unittest


ROOT = Path(__file__).resolve().parents[2]
SCRIPT_PATH = Path(__file__).with_name(
    "materialize_artist_31470_portable_particle_carriers.py"
)
FOUR_CLASS_PATH = Path(__file__).with_name(
    "materialize_four_class_track_a_candidates.py"
)
CODEC_PATH = ROOT / "Client/Private/Effect_DocumentCodec.cpp"
PLAYBACK_PATH = ROOT / "Client/Private/Effect_Playback.cpp"
RENDERER_PATH = ROOT / "Client/Private/Effect_DocumentRenderer.cpp"
DM_A_PATH = ROOT / (
    "Data/Effects/Imported/DimensionMaster/Converted/"
    "effect.dimensionmaster.skill.2050210.imported.effect.json"
)
DM_R_PATH = ROOT / (
    "Data/Effects/Imported/DimensionMaster/Converted/"
    "effect.dimensionmaster.skill.2050220.imported.effect.json"
)

sys.path.insert(0, str(SCRIPT_PATH.parent))
SPEC = importlib.util.spec_from_file_location("portable_particle", SCRIPT_PATH)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


def load(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def element(document: dict, element_id: str) -> dict:
    return next(row for row in document["elements"] if row["id"] == element_id)


class PortableParticleRuntimeCapabilityTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.dm_a = load(DM_A_PATH)
        cls.dm_r = load(DM_R_PATH)
        cls.codec = CODEC_PATH.read_text(encoding="utf-8-sig")
        cls.playback = PLAYBACK_PATH.read_text(encoding="utf-8-sig")
        cls.renderer = RENDERER_PATH.read_text(encoding="utf-8-sig")

    def test_codec_and_materializer_capability_tables_match(self) -> None:
        classes = re.search(
            r"PORTABLE_AUTHORED_PARTICLE_MODULE_CLASSES\s*=\s*\{(.*?)\};",
            self.codec,
            re.S,
        )
        self.assertIsNotNone(classes)
        codec_classes = set(re.findall(r'"([a-z0-9_]+)"', classes.group(1)))
        self.assertEqual(codec_classes, MODULE.PORTABLE_MODULE_CLASSES)

        maxima = re.search(
            r"PORTABLE_AUTHORED_PARTICLE_MODULE_MAX_COUNTS\s*=\s*\{(.*?)\};",
            self.codec,
            re.S,
        )
        self.assertIsNotNone(maxima)
        codec_maxima = {
            name: int(count)
            for name, count in re.findall(
                r'std::pair\{\s*"([a-z0-9_]+)",\s*([0-9]+)u\s*\}',
                maxima.group(1),
            )
        }
        self.assertEqual(codec_maxima, MODULE.PORTABLE_MODULE_MAX_COUNTS)
        for admitted in (
            "particlemoduleeventgenerator",
            "particlemoduleeventreceiverspawn",
            "particlemodulelocationcirclesurface",
            "particlemoduleorbit",
            "particlemodulesizescale",
            "particlemodulevectorfieldscale",
            "particlemodulevelocityinheritparent",
            "particlemodulevortex",
        ):
            self.assertIn(admitted, codec_classes)
        self.assertNotIn("particlemoduletypedataribbon", codec_classes)
        for blocked in (
            "particlemodulecollision",
            "particlemodulelocationemitter",
            "particlemodulemeshmaterial",
            "particlemodulesizemultiplyvelocity",
            "particlemodulesubuvmovie",
        ):
            self.assertNotIn(blocked, codec_classes)

    def test_every_admitted_module_has_a_playback_consumer(self) -> None:
        compact = re.sub(r"\s+", " ", self.playback)
        consumers = {
            "particlemoduleacceleration":
                'SourceClass_Matches(Module, "particlemoduleacceleration")',
            "particlemodulecameraoffset":
                'SourceClass_Matches(Module, "particlemodulecameraoffset")',
            "particlemodulecolor":
                'SourceClass_Matches(Module, "particlemodulecolor")',
            "particlemodulecoloroverlife":
                'Module, "particlemodulecoloroverlife"',
            "particlemodulecolorscaleoverlife":
                'Module, "particlemodulecolorscaleoverlife"',
            "particlemoduleeventgenerator":
                'Module, "particlemoduleeventgenerator"',
            "particlemoduleeventreceiverspawn":
                '"particlemoduleeventreceiverspawn"',
            "particlemodulelifetime":
                'SourceClass_Matches(Module, "particlemodulelifetime")',
            "particlemodulelocation":
                'SourceClass_Matches(Module, "particlemodulelocation")',
            "particlemodulelocationcirclesurface":
                'Module, "particlemodulelocationcirclesurface"',
            "particlemodulelocationdirect":
                'Module, "particlemodulelocationdirect"',
            "particlemodulelocalvectorfield":
                'Module, "particlemodulelocalvectorfield"',
            "particlemodulelocationonground":
                'Module, "particlemodulelocationonground"',
            "particlemodulelocationprimitivecylinder":
                'Module, "particlemodulelocationprimitivecylinder"',
            "particlemodulelocationprimitivecylinderspin":
                'Module, "particlemodulelocationprimitivecylinderspin"',
            "particlemodulelocationprimitivesphere":
                'Module, "particlemodulelocationprimitivesphere"',
            "particlemodulemeshrotation":
                'Module, "particlemodulemeshrotation"',
            "particlemodulemeshrotationrate":
                'Module, "particlemodulemeshrotationrate"',
            "particlemodulemeshrotationratemultiplylife":
                'Module, "particlemodulemeshrotationratemultiplylife"',
            "particlemodulemeshrotationrateoverlife":
                'Module, "particlemodulemeshrotationrateoverlife"',
            "particlemoduleorientationaxislock":
                'Module, "particlemoduleorientationaxislock"',
            "particlemoduleorbit":
                'SourceClass_Matches(Module, "particlemoduleorbit")',
            "particlemoduleparameterdynamic":
                'Module, "particlemoduleparameterdynamic"',
            "particlemodulerequired":
                'SourceClass_Matches(Module, "particlemodulerequired")',
            "particlemodulerotation":
                'SourceClass_Matches(Module, "particlemodulerotation")',
            "particlemodulerotationrate":
                'Module, "particlemodulerotationrate"',
            "particlemodulerotationratemultiplylife":
                'Module, "particlemodulerotationratemultiplylife"',
            "particlemodulesize":
                'SourceClass_Matches(Module, "particlemodulesize")',
            "particlemodulesizescale":
                'SourceClass_Matches(Module, "particlemodulesizescale")',
            "particlemodulesizescalebytime":
                'Module, "particlemodulesizescalebytime"',
            "particlemodulesizemultiplylife":
                'Module, "particlemodulesizemultiplylife"',
            "particlemodulespawn":
                'Find_SourceModule(Element, "particlemodulespawn")',
            "particlemodulespawnperunit":
                'Module, "particlemodulespawnperunit"',
            "particlemodulesubuv":
                'SourceClass_Matches(Module, "particlemodulesubuv")',
            "particlemoduletypedatamesh": "Is_MeshParticle(Element)",
            "particlemodulevectorfieldrotationrate":
                '"particlemodulevectorfieldrotationrate"',
            "particlemodulevectorfieldscale":
                'Module, "particlemodulevectorfieldscale"',
            "particlemodulevectorfieldscaleoverlife":
                '"particlemodulevectorfieldscaleoverlife"',
            "particlemodulevelocity":
                'SourceClass_Matches(Module, "particlemodulevelocity")',
            "particlemodulevelocityinheritparent":
                'Module, "particlemodulevelocityinheritparent"',
            "particlemodulevelocityoverlifetime":
                'Module, "particlemodulevelocityoverlifetime"',
            "particlemodulevortex":
                'SourceClass_Matches(Module, "particlemodulevortex")',
        }
        self.assertEqual(set(consumers), MODULE.PORTABLE_MODULE_CLASSES)
        for module_class, fragment in consumers.items():
            with self.subTest(module_class=module_class):
                self.assertIn(fragment, compact)

    def test_playback_proves_every_new_execution_semantic(self) -> None:
        required_fragments = (
            'bEffectAcceleration ? "acceldata" : "acceleration"',
            'Module, "particlemodulemeshrotation"',
            "Particle.vSourceMeshRotationDegrees = Add3(",
            'Module, "particlemodulerotationratemultiplylife"',
            "fYawOrSpriteRollRateScale *= Evaluate_ModuleFloat(",
            'Module, "particlemodulesizemultiplylife"',
            "Particle.vSize = Multiply3(Particle.vSize,",
            'Module, "particlemodulesizescalebytime"',
            'State, Module, "sizescalebytime", Particle.fAgeSeconds,',
            'Module, "particlemodulelocalvectorfield"',
            '"particlemodulevectorfieldrotationrate"',
            '"particlemodulevectorfieldscaleoverlife"',
            "Load_VectorFieldFromDisk(AssetId)",
            "Is_SourceNullCdoDistribution(*pRateScaleDistribution)",
            "fRate * fRateScale",
            "Validate_PortableSourceEventRoutes(",
            'Type != "epet_spawn"',
            "Portable source event queue overflow rolled back the fixed step.",
            'SourceString(Module, "surfaceaxis")',
            'Module, "particlemodulevelocityinheritparent"',
            "Particle.vInheritedParentVelocity",
            'Module, "particlemodulevectorfieldscale"',
            "Particle.fVectorFieldScale * fPerParticleScale",
            'State, Module, "sizescale", fNormalizedAge,',
            '"offsetoptions.bprocessduringupdate", false',
            'SourceNumber(Module, "power", 1.f) *',
        )
        for fragment in required_fragments:
            self.assertIn(fragment, self.playback)
        self.assertIn("m_bSourceEventQueueOverflow = true;", self.playback)

    def test_size_scale_by_time_uses_source_particle_seconds(self) -> None:
        row = element(
            self.dm_a,
            "fx_pc_swp_00.par_j_swp_willowrend_rb_00_1.particlespriteemitter_2",
        )
        lifetime = next(
            module
            for module in row["sourceRecipe"]["modules"]
            if module["className"] == "particlemodulelifetime"
        )["distributions"][0]
        size_scale = next(
            module
            for module in row["sourceRecipe"]["modules"]
            if module["className"] == "particlemodulesizescalebytime"
        )["distributions"][0]
        lifetime_seconds = float(lifetime["lookupTable"][2])
        entry_count = (
            len(size_scale["lookupTable"]) - 2
        ) // size_scale["lookupTableChunkSize"]
        source_time_span = (entry_count - 1) / size_scale["lookupTableTimeScale"]
        self.assertAlmostEqual(lifetime_seconds, 0.2)
        self.assertAlmostEqual(source_time_span, lifetime_seconds)
        self.assertRegex(
            self.playback,
            r'"sizescalebytime",\s*Particle\.fAgeSeconds,',
        )
        self.assertNotRegex(
            self.playback,
            r'"sizescalebytime",\s*fNormalizedAge,',
        )

    def test_rate_scale_preserves_null_cdo_identity_and_real_curves(self) -> None:
        rate_scales = []
        for row in self.dm_a["elements"]:
            if row["kind"] != "particle":
                continue
            spawn = next(
                (
                    module
                    for module in row["sourceRecipe"]["modules"]
                    if module["className"] == "particlemodulespawn"
                ),
                None,
            )
            if spawn is None:
                continue
            rate_scales.append(
                next(
                    distribution
                    for distribution in spawn["distributions"]
                    if distribution["propertyPath"] == "ratescale"
                )
            )
        self.assertTrue(any(MODULE.is_null_cdo_distribution(row) for row in rate_scales))
        self.assertTrue(any(not MODULE.is_null_cdo_distribution(row) for row in rate_scales))
        self.assertIn(
            "Is_SourceNullCdoDistribution(*pRateScaleDistribution)",
            self.playback,
        )

    def test_dm_a_full_100_closes_to_only_ribbon(self) -> None:
        particles = [row for row in self.dm_a["elements"] if row["kind"] == "particle"]
        self.assertEqual(len(particles), 100)
        portable = 0
        blockers: dict[str, int] = {}
        for row in particles:
            try:
                MODULE.portable_recipe(row["sourceRecipe"])
            except MODULE.MaterializeError as error:
                blockers[str(error)] = blockers.get(str(error), 0) + 1
            else:
                portable += 1
        self.assertEqual(portable, 99)
        self.assertEqual(
            blockers,
            {
                "unsupported source module class: particlemoduletypedataribbon": 1,
            },
        )

    def test_real_capability_carriers_and_non_null_rate_scale_are_admitted(self) -> None:
        admitted_ids = (
            "fx_pc_swp_00.par_j_swp_willowrend_core_00_1.particlespriteemitter_1",
            "fx_pc_swp_00.par_j_swp_willowrend_core_00_1.particlespriteemitter_23",
            "fx_pc_swp_00.par_j_swp_willowrend_rb_00_1.particlespriteemitter_5",
            "fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1.particlespriteemitter_9",
            "fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1.particlespriteemitter_13",
            "fx_pc_swp_00.par_j_swp_willowrend_swingdeco_00_1.particlespriteemitter_46",
        )
        for element_id in admitted_ids:
            staged = MODULE.portable_recipe(
                element(self.dm_a, element_id)["sourceRecipe"]
            )
            self.assertEqual(staged["emitterDelaySeconds"], 0)

        vector_field = element(
            self.dm_r,
            "fx_pc_swp_02.par_r_swp_momentaryrift_00_05.particlespriteemitter_4",
        )
        MODULE.portable_recipe(vector_field["sourceRecipe"])

    def test_vectorfield_scale_requires_one_safe_local_field(self) -> None:
        source = element(
            self.dm_a,
            "fx_pc_swp_00.par_j_swp_willowrend_swingdeco_00_2.particlespriteemitter_37",
        )["sourceRecipe"]
        MODULE.portable_recipe(source)
        without_local_field = copy.deepcopy(source)
        without_local_field["modules"] = [
            row
            for row in without_local_field["modules"]
            if row["className"] != "particlemodulelocalvectorfield"
        ]
        with self.assertRaisesRegex(
            MODULE.MaterializeError,
            "vector field companion has no unique local field",
        ):
            MODULE.portable_recipe(without_local_field)

        unsafe = copy.deepcopy(source)
        local = next(
            row
            for row in unsafe["modules"]
            if row["className"] == "particlemodulelocalvectorfield"
        )
        asset = next(
            row
            for row in local["literals"]
            if row["propertyPath"] == "vectorfield.assetid"
        )
        asset["value"] = "Effect/../outside.wvectorfield"
        with self.assertRaisesRegex(
            MODULE.MaterializeError, "local vector field asset is missing or unsafe"
        ):
            MODULE.portable_recipe(unsafe)

    def test_observed_composition_bounds_remain_fail_closed(self) -> None:
        source = copy.deepcopy(
            element(
                self.dm_a,
                "fx_pc_swp_00.par_j_swp_willowrend_core_00_1.particlespriteemitter_1",
            )["sourceRecipe"]
        )
        colors = [
            row for row in source["modules"] if row["className"] == "particlemodulecolor"
        ]
        self.assertEqual(len(colors), 2)
        for index in range(2):
            duplicate = copy.deepcopy(colors[0])
            duplicate["stableId"] += f".overflow{index}"
            source["modules"].append(duplicate)
        with self.assertRaisesRegex(
            MODULE.MaterializeError,
            "source module Family/cardinality is unsupported: particlemodulecolor",
        ):
            MODULE.portable_recipe(source)

    def test_observed_repeated_and_renderer_inert_modules_preserve_order(self) -> None:
        fixtures = (
            (
                "Data/Effects/Imported/Artist/CurrentCombat/Converted/"
                "effect.artist.skill.31200.imported.effect.json",
                "fx_pc_sdm_04.par_w_sdm_inkpaddle_02.particlespriteemitter_15",
                "particlemodulelocation",
                3,
            ),
            (
                "Data/Effects/Imported/Artist/CurrentCombat/Converted/"
                "effect.artist.skill.31210.imported.effect.json",
                "fx_pc_sdm_01.par_t_sdm_skykongkong_skl_01.particlespriteemitter_23",
                "particlemodulerotation",
                2,
            ),
            (
                "Data/Effects/Imported/Artist/CurrentCombat/Converted/"
                "effect.artist.skill.31430.imported.effect.json",
                "fx_pc_sdm_01.par_t_sdm_inkshot_skl_01.particlespriteemitter_6",
                "particlemodulemeshrotation",
                1,
            ),
            (
                "Data/Effects/Imported/DimensionMaster/Converted/"
                "effect.dimensionmaster.skill.2050220.imported.effect.json",
                "fx_pc_swp_02.par_r_swp_momentaryrift_00_02.particlespriteemitter_3",
                "particlemodulevelocityoverlifetime",
                2,
            ),
            (
                "Data/Effects/Imported/Artist/CurrentCombat/Converted/"
                "effect.artist.skill.31930.imported.effect.json",
                "fx_pc_sdm_09.par_m_flowergarden_exp_01.particlespriteemitter_4",
                "particlemodulerotationrate",
                2,
            ),
            (
                "Data/Effects/Imported/Artist/CurrentCombat/Converted/"
                "effect.artist.skill.31930.imported.effect.json",
                "fx_pc_sdm_09.par_m_flowergarden_field_02.particlespriteemitter_36",
                "particlemoduleorientationaxislock",
                1,
            ),
            (
                "Data/Effects/Imported/DimensionMaster/Converted/"
                "effect.dimensionmaster.skill.2050010.imported.effect.json",
                "fx_pc_swp_00.par_j_swp_normalatk_core_3_1.particlespriteemitter_15",
                "particlemodulemeshrotationrate",
                2,
            ),
            (
                "Data/Effects/Imported/DimensionMaster/Converted/"
                "effect.dimensionmaster.skill.2050230.imported.effect.json",
                "fx_pc_swp_03.par_s_swp_chrono_rewind_02.particlespriteemitter_39",
                "particlemodulelocationprimitivesphere",
                2,
            ),
            (
                "Data/Effects/Imported/DimensionMaster/Converted/"
                "effect.dimensionmaster.skill.2050240.imported.effect.json",
                "fx_pc_swp_01.par_w_swp_flickerthrust_01.particlespriteemitter_64",
                "particlemodulelifetime",
                2,
            ),
            (
                "Data/Effects/Imported/DimensionMaster/Converted/"
                "effect.dimensionmaster.skill.2050500.imported.effect.json",
                "fx_pc_swp_01.par_w_swp_dimensionprison_hand.particlespriteemitter_6",
                "particlemodulecameraoffset",
                2,
            ),
            (
                "Data/Effects/Imported/LanceMaster/CurrentCombat/Converted/"
                "effect.lancemaster.skill.34630.imported.effect.json",
                "fx_pc_flm_09.par_s_flm_superlance_atk_04.particlespriteemitter_22",
                "particlemodulelocationprimitivecylinder",
                2,
            ),
        )
        for relative_path, element_id, module_class, expected_count in fixtures:
            with self.subTest(element_id=element_id, module_class=module_class):
                source = element(load(ROOT / relative_path), element_id)[
                    "sourceRecipe"
                ]
                source_ids = [row["stableId"] for row in source["modules"]]
                staged = MODULE.portable_recipe(source)
                self.assertEqual(
                    [row["stableId"] for row in staged["modules"]], source_ids
                )
                self.assertEqual(
                    sum(
                        MODULE.normalized_module_class(row["className"])
                        == module_class
                        for row in staged["modules"]
                    ),
                    expected_count,
                )

                overflow = copy.deepcopy(source)
                matching = next(
                    row
                    for row in overflow["modules"]
                    if MODULE.normalized_module_class(row["className"])
                    == module_class
                )
                current_count = sum(
                    MODULE.normalized_module_class(row["className"])
                    == module_class
                    for row in overflow["modules"]
                )
                maximum = MODULE.PORTABLE_MODULE_MAX_COUNTS.get(module_class, 1)
                for index in range(maximum - current_count + 1):
                    duplicate = copy.deepcopy(matching)
                    duplicate["stableId"] += f".overflow{index}"
                    overflow["modules"].append(duplicate)
                with self.assertRaisesRegex(
                    MODULE.MaterializeError,
                    f"source module Family/cardinality is unsupported: {module_class}",
                ):
                    MODULE.portable_recipe(overflow)

        compact_playback = re.sub(r"\s+", " ", self.playback)
        self.assertGreaterEqual(
            compact_playback.count(
                "for (const EFFECT_SOURCE_MODULE_DESC& Module : Element.SourceRecipe.Modules)"
            ),
            2,
        )
        self.assertIn(
            "Out.Modules.push_back(std::move(Module));", self.codec
        )
        self.assertIn(
            "const matrix_t ParticleRotation = bMeshParticle ?", self.playback
        )
        self.assertIn(
            "if (nullptr != pResource->pModel)", self.renderer
        )
        self.assertIn("Make_ParticleSpriteWorld(Particle, World)", self.renderer)

    def test_source_without_size_uses_validated_flattened_detail_fallback(self) -> None:
        source_element = element(
            load(
                ROOT
                / "Data/Effects/Imported/Artist/CurrentCombat/Converted/"
                "effect.artist.skill.31430.imported.effect.json"
            ),
            "fx_pc_sdm_01.par_t_sdm_inkshot_skl_01.particlespriteemitter_13",
        )
        source = source_element["sourceRecipe"]
        self.assertFalse(
            any(
                MODULE.normalized_module_class(row["className"])
                == "particlemodulesize"
                for row in source["modules"]
            )
        )
        start_size = source_element["detail"]["particle"]["startSize"]
        self.assertEqual(len(start_size), 2)
        self.assertTrue(all(float(value) > 0.0 for value in start_size))
        MODULE.portable_recipe(source)
        self.assertIn("if (!bHasSize)", self.playback)
        self.assertIn("Desc.vStartSize.x, Desc.vStartSize.y", self.playback)

    def test_four_class_strict_and_full_source_denominators_are_explicit(self) -> None:
        spec = importlib.util.spec_from_file_location(
            "four_class_capability_denominator", FOUR_CLASS_PATH
        )
        assert spec is not None and spec.loader is not None
        four_class = importlib.util.module_from_spec(spec)
        sys.modules[spec.name] = four_class
        spec.loader.exec_module(four_class)
        _, receipt = four_class.build_projection()
        self.assertEqual(receipt["counts"]["strictMappedParticleCount"], 4687)
        self.assertEqual(receipt["counts"]["portableCount"], 4488)
        self.assertEqual(
            receipt["counts"]["sourcePreservedDeferredCount"], 199
        )
        by_class = {}
        for row in receipt["targets"]:
            bucket = by_class.setdefault(
                row["characterClass"], {"portable": 0, "deferred": 0}
            )
            bucket["portable"] += row["portableCount"]
            bucket["deferred"] += row["sourcePreservedDeferredCount"]
        self.assertEqual(
            by_class,
            {
                "ARTIST": {"portable": 540, "deferred": 114},
                "DIMENSIONMASTER": {"portable": 767, "deferred": 17},
                "LANCE_MASTER": {"portable": 1984, "deferred": 34},
                "WARLORD": {"portable": 1197, "deferred": 34},
            },
        )
        source_by_effect, _ = four_class.load_source_index()
        particles = [
            row
            for source in source_by_effect.values()
            for row in source.document["elements"]
            if row["kind"] == "particle"
        ]
        portable = 0
        for row in particles:
            try:
                MODULE.portable_recipe(row["sourceRecipe"])
            except MODULE.MaterializeError:
                pass
            else:
                portable += 1
        self.assertEqual(len(particles), 4846)
        self.assertEqual(portable, 4641)
        self.assertEqual(len(particles) - portable, 205)


if __name__ == "__main__":
    unittest.main()
