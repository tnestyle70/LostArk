import json
import unittest
from pathlib import Path


class DimensionMaster2050010ThreeClickTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.repository_root = Path(__file__).resolve().parents[2]

    def load_json(self, relative_path: str) -> dict:
        return json.loads((self.repository_root / relative_path).read_text(encoding="utf-8"))

    def load_effect(self, stage: int, full: bool = False) -> dict:
        suffix = "full.restore" if full else "restore"
        return self.load_json(
            "Data/Effects/Authored/"
            f"effect.dimensionmaster.skill.2050010.ba{stage}.{suffix}.effect.json"
        )

    def test_three_clicks_preserve_first_clip_double_thrust_and_final_slashes(self) -> None:
        bindings = self.load_json(
            "Data/Animation/Authored/DimensionMaster/DimensionMaster.skillbindings.json"
        )["bindings"]
        binding = next(row for row in bindings if row["skillId"] == 2050010)
        self.assertEqual(binding["clips"], [
            [{"clip": "pc_sp_m_00_sk_att_battle_1_01", "playMs": 1400, "playRate": 2.0}],
            [{"clip": "pc_sp_m_00_sk_att_battle_1_03", "playRate": 2.0}],
            [{"clip": "pc_sp_m_00_sk_att_battle_1_04", "playRate": 2.0}],
        ])
        skill = next(row for row in self.load_json("Data/Balance/PlayerSkills.json")["skills"]
                     if row["skillId"] == 2050010)
        stages = skill["comboStages"]
        self.assertEqual((skill["actionDurationMs"], skill["hitTimeMs"]), (1400, 100))
        self.assertEqual([s["actionDurationMs"] for s in stages], [1400, 1067, 1700])
        self.assertEqual([s["hitTimeMs"] for s in stages], [100, 28, 335])
        self.assertEqual([(s["inputOpenMs"], s["inputCloseMs"]) for s in stages],
                         [(100, 1400), (93, 1067), (0, 0)])
        for s in stages:
            self.assertEqual(s["comboAdvanceMs"], s["actionDurationMs"])
        # The existing server interprets a nonfinal 0/0 window as automatic.
        # Every nonfinal BA must instead require another click or hold repeat.
        self.assertTrue(all(s["inputCloseMs"] > s["inputOpenMs"] for s in stages[:-1]))
        notify = (self.repository_root / "Data/Animation/Reference/DimensionMaster/"
                  "DimensionMaster.animnotify").read_text(encoding="utf-8")
        first_clip = notify.split('"pc_sp_m_00_sk_att_battle_1_01"', 1)[1].split(
            '"pc_sp_m_00_sk_att_battle_1_02"', 1)[0]
        thrust_rows = [line for line in first_clip.splitlines()
                       if 'asset="FX_PC_SWP_00.Par_J_SWP_NormalAtk_0_1"' in line]
        self.assertEqual([float(line.split("t=", 1)[1].split()[0]) for line in thrust_rows], [.2, .4])
        self.assertIn('t=1.4000 d=2.6000 kind=CANCEL', first_clip)

    def test_product_cues_connect_each_clip_to_its_own_full_effect(self) -> None:
        lines = (self.repository_root / "Data/Animation/Authored/DimensionMaster/"
                 "DimensionMaster.animevents").read_text(encoding="utf-8").splitlines()
        self.assertEqual(int(lines[0].rsplit(" ", 1)[1]), len(lines) - 1)
        cues = [line for line in lines if 'payload="effect.dimensionmaster.skill.2050010.ba' in line]
        self.assertEqual(len(cues), 4)
        for stage, line in enumerate(cues):
            self.assertTrue(line.startswith(f'"pc_sp_m_00_sk_att_battle_1_0{stage + 1}" EFFECT startms=0 '))
            self.assertIn(f'payload="effect.dimensionmaster.skill.2050010.ba{stage}.full.restore"', line)
            self.assertIn('effectref=asset anchor="root" follow=follow orientation=action_facing stop=natural', line)
        self.assertTrue(any('"pc_sp_m_00_sk_att_battle_1_02" EFFECT' in line and 'src=orig' in line
                            for line in lines))

    def test_tuned_and_full_product_documents_have_distinct_payloads(self) -> None:
        rows = {r["effectAssetId"]: r for r in self.load_json("Data/Effects/EffectCatalog.json")["effects"]}
        for stage in range(4):
            for full in [False, True]:
                effect = self.load_effect(stage, full)
                asset = effect["effectAssetId"]
                self.assertIn(asset, rows, f"Missing Catalog row: {asset}")
                self.assertEqual(rows[asset]["payloadKind"], "DIRECT_AUTHORED_DOCUMENT")
                self.assertEqual(rows[asset]["authoringPath"], f"Effects/Authored/{asset}.effect.json")
                self.assertEqual((effect["schema"], effect["version"]), ("lostark.effect-authoring", 13))
            self.assertNotEqual(self.load_effect(stage)["elements"], self.load_effect(stage, True)["elements"])

    def test_legacy_payloads_keep_their_actual_clip_owners_as_auditions(self) -> None:
        import hashlib
        rows = {r["effectAssetId"]: r for r in self.load_json("Data/Effects/EffectAuditionCatalog.json")["effects"]}
        product = {r["effectAssetId"] for r in self.load_json("Data/Effects/EffectCatalog.json")["effects"]}
        for old, stage in {1: 3, 2: 0, 3: 2}.items():
            legacy = f"effect.dimensionmaster.skill.2050010.ba{old}.unified"
            source = f"effect.dimensionmaster.skill.2050010.ba{stage}.restore"
            self.assertNotIn(legacy, product)
            self.assertEqual(rows[legacy]["sourceEffectAssetId"], source)
            self.assertEqual(rows[legacy]["runtimeAdmission"], "REGISTRY_BOUND_AUDITION_ONLY")
            source_path = Path(__file__).resolve().parents[2] / f"Data/Effects/Authored/{source}.effect.json"
            self.assertEqual(rows[legacy]["sourceDocumentRawSha256"], hashlib.sha256(source_path.read_bytes()).hexdigest())

    def test_tuned_thrust_reuses_native_51_and_swing_uses_real_mesh_mask(self) -> None:
        for e in self.load_effect(0)["elements"]:
            profile = e["material"]["sourceProfile"]
            self.assertEqual(profile["runtimeShaderProfileId"], "effect.ue3.q-glass-hole-native.v1")
            aura = next(v["value"] for v in profile["vectors"] if v["name"] == "aura_color")
            self.assertGreater(aura[2], aura[1])
            self.assertGreater(aura[0], aura[1])
            self.assertTrue(e["sourceRecipe"]["authoredModuleOverrides"])
            self.assertTrue(any(m["className"] == "particlemoduleparameterdynamic"
                                for m in e["sourceRecipe"]["modules"]))
        self.assertEqual([e["detail"]["timing"]["startDelaySeconds"] for e in self.load_effect(0)["elements"]], [.2, .4])
        for stage in range(1, 4):
            for e in self.load_effect(stage)["elements"]:
                resources = {r["slotId"]: r["assetId"] for r in e["resources"]}
                self.assertIn("fm_h_swing_", resources["meshModel"])
                self.assertIn("mask", resources)
                self.assertEqual(e["detail"]["mesh"]["modelPreScale"], .01)
                self.assertFalse(e["detail"]["particle"]["billboard"])

    def test_full_documents_preserve_admitted_native_occurrences(self) -> None:
        all_ids = []
        # Native material programs close the 16 previously omitted source rows.
        for stage, count in enumerate([18, 14, 9, 34]):
            effect = self.load_effect(stage, True)
            self.assertEqual(len(effect["elements"]), count)
            for e in effect["elements"]:
                all_ids.append(e["id"])
                self.assertIn("|element:", e["sourceNode"])
                self.assertTrue(e["sourceRecipe"]["enabled"])
                for resource in e["resources"]:
                    path = self.repository_root / "Client/Bin/Resources" / resource["assetId"]
                    self.assertTrue(path.is_file(), resource["assetId"])
                if e["kind"] == "light":
                    self.assertTrue(e["visible"])
                    self.assertTrue(e["detail"]["light"]["enabled"])
                    self.assertEqual(e["detail"]["light"]["range"], 2)
                    self.assertEqual(e["detail"]["light"]["intensity"], 10)
                else:
                    profile = e["material"]["sourceProfile"]
                    self.assertTrue(profile["enabled"])
                    self.assertIn("native.v1", profile["runtimeShaderProfileId"])
                    for texture in profile["textures"]:
                        self.assertTrue((self.repository_root / "Client/Bin/Resources" /
                                         texture["assetId"]).is_file(), texture["assetId"])
                    self.assertEqual(e["detail"]["uv"]["speed"], [0, 0])
                    self.assertEqual(e["detail"]["color"]["emissiveIntensity"], 1)
        self.assertEqual(len(all_ids), len(set(all_ids)))

    def test_root_motion_follows_the_same_three_selected_clips_without_retiming(self) -> None:
        skill = next(row for row in self.load_json(
            "Data/Animation/RootMotion/DimensionMaster.rootmotion.json")["skills"]
                     if row["skillId"] == 2050010)
        stages = skill["stages"]
        self.assertEqual([s["durationMs"] for s in stages], [1400, 1067, 1700])
        self.assertEqual([len(s["samples"]) for s in stages], [43, 33, 52])
        for index, (stage, forward) in enumerate(zip(stages, [.8418, .2802, 1.0404])):
            self.assertEqual(stage["stageIndex"], index)
            times = [s["timeMs"] for s in stage["samples"]]
            self.assertEqual((times[0], times[-1]), (0, stage["durationMs"]))
            self.assertTrue(all(a < b for a, b in zip(times, times[1:])))
            self.assertAlmostEqual(stage["samples"][-1]["forward"], forward)


if __name__ == "__main__":
    unittest.main()
