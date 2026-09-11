import copy
import importlib.util
import json
import re
import tempfile
import unittest
from unittest import mock
from pathlib import Path


class CharacterSkillEffectCanonicalSelectionTests(unittest.TestCase):
    ARTIST_EFFECT = "effect.artist.skill.31460.linear-reveal.unified"
    WARLORD_EFFECT = "effect.warlord.skill.17080.clip2.unified"

    @classmethod
    def setUpClass(cls) -> None:
        cls.repository_root = Path(__file__).resolve().parents[2]
        cls.player_skills = cls.load_json("Data/Balance/PlayerSkills.json")[
            "skills"
        ]
        cls.catalog_rows = {
            row["effectAssetId"]: row
            for row in cls.load_json("Data/Effects/EffectCatalog.json")["effects"]
        }

    @classmethod
    def load_json(cls, relative_path: str) -> dict:
        return json.loads(
            (cls.repository_root / relative_path).read_text(encoding="utf-8")
        )

    @classmethod
    def load_binding(cls, asset_name: str, skill_id: int) -> dict:
        document = cls.load_json(
            f"Data/Animation/Authored/{asset_name}/{asset_name}.skillbindings.json"
        )
        return next(
            row for row in document["bindings"] if row["skillId"] == skill_id
        )

    @classmethod
    def collect_product_cues(
        cls, asset_name: str, clip_names: set[str]
    ) -> list[tuple[str, int, str]]:
        event_path = (
            cls.repository_root
            / f"Data/Animation/Authored/{asset_name}/{asset_name}.animevents"
        )
        cue_pattern = re.compile(
            r'^"([^"]+)" EFFECT startms=(\d+) payload="([^"]+)" '
            r'effectref=asset\b'
        )
        cues: list[tuple[str, int, str]] = []
        for line in event_path.read_text(encoding="utf-8").splitlines():
            match = cue_pattern.match(line)
            if match is None or match.group(1) not in clip_names:
                continue
            cues.append((match.group(1), int(match.group(2)), match.group(3)))
        return cues

    def test_player_skill_owners_and_stage_shapes_remain_authoritative(self) -> None:
        artist = next(row for row in self.player_skills if row["skillId"] == 31460)
        warlord = next(row for row in self.player_skills if row["skillId"] == 17080)

        self.assertEqual(
            (artist["characterClass"], artist["inputSlot"], artist["skillKind"]),
            ("ARTIST", "A", "ACTIVE"),
        )
        self.assertEqual(
            (warlord["characterClass"], warlord["inputSlot"], warlord["skillKind"]),
            ("WARLORD", "E", "COMBO"),
        )
        self.assertEqual(artist["effectId"], self.ARTIST_EFFECT)
        self.assertEqual(warlord["effectId"], self.WARLORD_EFFECT)
        self.assertEqual(len(warlord["comboStages"]), 2)

    def test_skillbindings_keep_the_selected_animation_occurrences(self) -> None:
        artist = self.load_binding("Artist", 31460)
        warlord = self.load_binding("Warlord", 17080)

        self.assertEqual(artist["clips"], ["sdm_sk_butterflydream"])
        self.assertEqual(
            warlord["clips"],
            [
                ["wgl_sk_dashupperfire_01"],
                ["wgl_sk_dashupperfire_02"],
            ],
        )

    def test_artist_a_uses_only_the_selected_linear_reveal_product(self) -> None:
        self.assertEqual(
            self.collect_product_cues("Artist", {"sdm_sk_butterflydream"}),
            [("sdm_sk_butterflydream", 0, self.ARTIST_EFFECT)],
        )

    def test_warlord_e_both_combo_occurrences_reuse_clip2_product(self) -> None:
        self.assertEqual(
            self.collect_product_cues(
                "Warlord",
                {"wgl_sk_dashupperfire_01", "wgl_sk_dashupperfire_02"},
            ),
            [
                ("wgl_sk_dashupperfire_01", 0, self.WARLORD_EFFECT),
                ("wgl_sk_dashupperfire_02", 0, self.WARLORD_EFFECT),
            ],
        )

    def test_lancemaster_document_count_and_combined_alt_v_admission(self) -> None:
        path = self.repository_root / "Data/Animation/Authored/LanceMaster/LanceMaster.animevents"
        lines = path.read_text(encoding="utf-8").splitlines()
        header = re.fullmatch(r'LOSTARK_ANIM_EVENTS [3-6] "LanceMaster" (\d+)', lines[0])
        self.assertIsNotNone(header)
        binary_rows = path.read_bytes().split(b"\n")[1:]
        self.assertFalse(any(row and not row.strip() for row in binary_rows))
        self.assertEqual(int(header[1]), sum(bool(row) for row in binary_rows))
        clips = {f"flm_sk_super_squalllance_0{i}" for i in range(1, 5)}
        self.assertEqual(self.collect_product_cues("LanceMaster", clips),
                         [("flm_sk_super_squalllance_01", 0,
                           "effect.lancemaster.skill.34630.full.restore")])

    def test_selected_effects_are_direct_authored_catalog_definitions(self) -> None:
        for effect_asset_id in (self.ARTIST_EFFECT, self.WARLORD_EFFECT):
            row = self.catalog_rows[effect_asset_id]
            self.assertEqual(row["payloadKind"], "DIRECT_AUTHORED_DOCUMENT")
            expected_authoring_path = (
                f"Effects/Authored/{effect_asset_id}.effect.json"
            )
            self.assertEqual(row["authoringPath"], expected_authoring_path)

            document = self.load_json(f"Data/{expected_authoring_path}")
            self.assertEqual(document["effectAssetId"], effect_asset_id)
            self.assertTrue(any(element["visible"] for element in document["elements"]))


class LanceMasterAltVProductCueMigrationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        path = Path(__file__).with_name("combine_lancemaster_altv_full_restore.py")
        spec = importlib.util.spec_from_file_location("combine_lancemaster_altv", path)
        cls.combine = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(cls.combine)

    def original_events(self):
        rows = [b'"other" SOUND startms=25 payload="preserved"']
        for index, clip in enumerate(self.combine.CLIPS, 1):
            rows.append((f'"{clip}" EFFECT startms=0 '
                         f'payload="effect.lancemaster.skill.34630.clip{index}.full.restore" '
                         'effectref=asset anchor="root" px=1.25 ry=30').encode())
        return b'LOSTARK_ANIM_EVENTS 5 "LanceMaster" 5\r\n' + b'\r\n'.join(rows) + b'\r\n'

    def test_collapse_preserves_other_rows_tuning_newlines_and_count(self):
        original = self.original_events()
        result = self.combine.retarget_product_events(original)
        self.assertTrue(result.startswith(b'LOSTARK_ANIM_EVENTS 5 "LanceMaster" 2\r\n'))
        self.assertIn(b'"other" SOUND startms=25 payload="preserved"\r\n', result)
        self.assertIn(b'anchor="root" px=1.25 ry=30\r\n', result)
        self.assertEqual(result.count(b' EFFECT '), 1)
        self.assertEqual(result, self.combine.retarget_product_events(result))
        self.assertEqual(result.count(b'\n'), result.count(b'\r\n'))

    def test_lf_empty_rows_are_preserved_but_whitespace_rows_are_rejected(self):
        original = self.original_events().replace(b"\r\n", b"\n")
        original = original.replace(b'payload="preserved"\n', b'payload="preserved"\n\n')
        result = self.combine.retarget_product_events(original)
        self.assertIn(b'payload="preserved"\n\n', result)
        self.assertEqual(self.combine.count_product_event_rows(result), 2)
        for row in (b"\r\n", b" \n", b"\t\n"):
            with self.subTest(row=row), self.assertRaises(ValueError):
                self.combine.retarget_product_events(self.original_events() + row)

    def test_full_input_validation_precedes_product_event_output(self):
        with mock.patch.object(self.combine, "read", return_value={"bindings": [{"skillId": 34630, "clips": []}]}), \
             mock.patch.object(self.combine, "write_product_events") as write, \
             mock.patch.object(self.combine.sys, "argv", ["combine", "--output-root", "unused-output"]):
            with self.assertRaisesRegex(ValueError, "four native clips"):
                self.combine.main()
            write.assert_not_called()

    def test_invalid_count_or_ambiguous_cues_are_rejected(self):
        original = self.original_events()
        combined = self.combine.retarget_product_events(original)
        cue = original.splitlines(keepends=True)[-1]
        invalid = [original.replace(b'"LanceMaster" 5', b'"LanceMaster" 8', 1),
                   original.replace(cue, b'').replace(b'"LanceMaster" 5', b'"LanceMaster" 4', 1),
                   (original + cue).replace(b'"LanceMaster" 5', b'"LanceMaster" 6', 1),
                   original.replace(b'px=1.25', b'px=9.25', 1),
                   original.replace(b'"flm_sk_super_squalllance_02" EFFECT', b'"wrong_clip" EFFECT'),
                   (combined + cue).replace(b'"LanceMaster" 2', b'"LanceMaster" 3', 1)]
        for data in invalid:
            with self.subTest(data=data), self.assertRaises(ValueError):
                self.combine.retarget_product_events(data)

    def test_atomic_output_preserves_source_and_existing_output_on_failure(self):
        relative = Path("Animation/Authored/LanceMaster/LanceMaster.animevents")
        with tempfile.TemporaryDirectory() as directory, mock.patch.object(self.combine, "ROOT", Path(directory)):
            root = Path(directory)
            source, output = root / "Data" / relative, root / "out" / relative
            source.parent.mkdir(parents=True)
            output.parent.mkdir(parents=True)
            original = self.original_events()
            source.write_bytes(original)
            output.write_bytes(b"previous output")
            with mock.patch.object(self.combine.os, "replace", side_effect=OSError("injected replace failure")):
                with self.assertRaises(OSError):
                    self.combine.write_product_events(root / "out")
            self.assertEqual(output.read_bytes(), b"previous output")
            self.assertEqual(source.read_bytes(), original)
            self.assertFalse(list(output.parent.glob("*.tmp")))
            invalid = original.replace(b'"LanceMaster" 5', b'"LanceMaster" 8', 1)
            source.write_bytes(invalid)
            with self.assertRaises(ValueError):
                self.combine.write_product_events(root / "out")
            self.assertEqual(output.read_bytes(), b"previous output")
            source.write_bytes(original)
            receipt = self.combine.write_product_events(root / "out")
            self.assertEqual(receipt["eventRows"], 2)
            self.assertEqual(output.read_bytes(), self.combine.retarget_product_events(original))
            self.assertEqual(source.read_bytes(), original)
            with self.assertRaises(ValueError):
                self.combine.write_product_events(root / "Data")
            self.assertEqual(source.read_bytes(), original)


class LanceSpearTypeDataProjectionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        path = Path(__file__).with_name("generate_lancemaster_all_full_restore_documents.py")
        spec = importlib.util.spec_from_file_location("lance_spear_generator", path)
        cls.generator = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(cls.generator)
        cls.authored = path.parents[2] / "Data/Effects/Authored"
        cls.names = [f"effect.lancemaster.skill.34610.clip{i}.full.restore.effect.json" for i in range(1, 4)]
        cls.names += [f"effect.lancemaster.skill.34630.clip{i}.full.restore.effect.json" for i in range(1, 5)]
        cls.names += ["effect.lancemaster.skill.34630.full.restore.effect.json"]
        cls.documents = [json.loads((cls.authored / name).read_bytes()) for name in cls.names]
        cls.targets = [[element for element in document["elements"] if any(
            resource.get("assetId") in cls.generator.LANCE_SPEAR_MESHES.values()
            for resource in element.get("resources", []))] for document in cls.documents]

    def test_exact_source_rotation_is_projected_once_in_all_23_rows(self):
        self.assertEqual([len(rows) for rows in self.targets], [1, 1, 1, 4, 2, 2, 2, 10])
        for rows in self.targets:
            for element in rows:
                with self.subTest(element=element["id"]):
                    self.assertEqual(self.generator.source_type_data_mesh_rotation_degrees(element), [0, -90, 0])
                    self.assertEqual(element["detail"]["mesh"]["sourceTypeDataRotationDegrees"], [0, -90, 0])
                    staged = copy.deepcopy(element)
                    self.assertFalse(self.generator.project_lance_spear_type_data_rotation(staged))
                    self.assertEqual(staged, element)
                    del staged["detail"]["mesh"]["sourceTypeDataRotationDegrees"]
                    self.assertTrue(self.generator.project_lance_spear_type_data_rotation(staged))
                    self.assertEqual(staged, element)

    def test_invalid_source_fields_fail_before_changing_any_element(self):
        base = self.targets[0][0]
        for mode in ("duplicate_pitch", "not_finite", "not_number", "wrong_mesh"):
            staged = copy.deepcopy(base)
            typed = next(module for module in staged["sourceRecipe"]["modules"]
                         if module["className"] == "particlemoduletypedatamesh")
            pitch = next(row for row in typed["literals"] if row["propertyPath"] == "pitch")
            if mode == "duplicate_pitch": typed["literals"].append(copy.deepcopy(pitch))
            elif mode == "not_finite": pitch["value"] = float("inf")
            elif mode == "not_number": pitch["kind"] = "string"
            else: next(row for row in typed["literals"] if row["propertyPath"] == "mesh.objectpath")["value"] = "wrong.mesh"
            original = copy.deepcopy(staged)
            with self.subTest(mode=mode), self.assertRaises((ValueError, RuntimeError)):
                self.generator.project_lance_spear_type_data_rotation(staged)
            self.assertEqual(staged, original)

    def test_existing_alt_v_repair_consumes_projection_without_tuning_other_fields(self):
        element = copy.deepcopy(self.targets[3][0])
        element["detail"]["mesh"]["sourceTypeDataRotationDegrees"] = [0, 0, 0]
        element["detail"]["transform"]["rotationDegrees"] = [12, 34, 56]
        element["actionCueAttachment"]["socketLocalTransform"]["position"] = [1, 2, 3]
        document = {"effectAssetId": "effect.lancemaster.skill.34630.clip1.full.restore", "elements": [element]}
        expected = copy.deepcopy(document)
        expected["elements"][0]["detail"]["mesh"]["sourceTypeDataRotationDegrees"] = [0, -90, 0]
        with tempfile.TemporaryDirectory() as directory, mock.patch.object(self.generator, "ROOT", Path(directory)):
            path = Path(directory) / "Data/Effects/Authored" / self.names[3]
            path.parent.mkdir(parents=True)
            path.write_text(json.dumps(document), encoding="utf-8")
            evidence = Path(directory) / "evidence"
            evidence.mkdir()
            self.generator.repair_altv_solo_documents(evidence)
            self.assertEqual(json.loads(path.read_bytes()), expected)
            saved = path.read_bytes()
            self.generator.repair_altv_solo_documents(evidence)
            self.assertEqual(path.read_bytes(), saved)

    def test_combined_spear_rows_only_shift_the_original_start_times(self):
        full = {element["id"]: element for element in self.targets[-1]}
        for rows, offset in zip(self.targets[3:7], (0, 2.234, 4.434, 5.434)):
            for element in rows:
                shifted = copy.deepcopy(element)
                shifted["detail"]["timing"]["startDelaySeconds"] += offset
                self.assertEqual(full[element["id"]], shifted)


if __name__ == "__main__":
    unittest.main()
