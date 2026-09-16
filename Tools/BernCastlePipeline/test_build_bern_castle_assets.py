from __future__ import annotations

import json
import shutil
import struct
import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import build_bern_castle_assets as pipeline
import cook_wmodel_geometry_contract as geometry
from test_cook_wmodel_geometry_contract import _write_source_gltf, _build_legacy_wmodel


class BernCastlePipelineTests(unittest.TestCase):
    def test_asset_id_uses_existing_sha1_contract(self):
        self.assertEqual(
            pipeline.stable_asset_id(
                "lv_navimesh.mesh.lv_common_mesh_cul_box_8",
                "lv_common_mesh_cul_box_8",
            ),
            "MAP_1E5F55FC0524_LV_COMMON_MESH_CUL_BOX_8",
        )

    def geometry_fixture(self, root, *, optional=True):
        asset = {"assetId": "FIXTURE", "fullPath": "fixture.mesh", "objectName": "fixture"}
        source = root / "source/FIXTURE"
        gltf = _write_source_gltf(source, "fixture", optional, with_uv1=optional)
        document = json.loads(gltf.read_text())
        document["materials"] = [{"name": "fixture"}]
        primitive = document["meshes"][0]["primitives"][0]
        primitive["material"] = 0
        if optional:
            buffer_path = source / "fixture.bin"
            payload = buffer_path.read_bytes()
            uv2 = struct.pack("<6f", .2, .3, .4, .5, .6, .7)
            document["bufferViews"].append({"buffer": 0, "byteOffset": len(payload), "byteLength": len(uv2)})
            document["accessors"].append({"bufferView": len(document["bufferViews"])-1,
                "componentType": 5126, "type": "VEC2", "count": 3})
            primitive["attributes"]["TEXCOORD_2"] = len(document["accessors"])-1
            buffer_path.write_bytes(payload + uv2)
            document["buffers"][0]["byteLength"] = len(payload + uv2)
        gltf.write_text(json.dumps(document), encoding="utf-8")
        package = root / "fixture.upk"; package.write_bytes(b"observed fixture package")
        converter = root / "converter.exe"; converter.write_bytes(b"fixture converter identity")
        self.refresh_source_receipt(source, asset, package)
        return asset, gltf, converter

    def refresh_source_receipt(self, source, asset, package):
        pipeline.atomic_write_json(source / "source.receipt.json", {
            "schemaVersion": 2, "assetId": asset["assetId"], "fullPath": asset["fullPath"],
            "gltf": "fixture.gltf", "physicalPackage": package.name,
            "physicalPackagePath": str(package), "materials": [],
            "outputs": [{"path": p.name, "sha256": pipeline.sha256(p)}
                        for p in sorted(source.iterdir()) if p.suffix in (".gltf", ".bin")],
        })

    def converter_stub(self, command, cwd, timeout, label):
        if "-o" in command:
            _build_legacy_wmodel(Path(command[command.index("-o") + 1]))
        return subprocess.CompletedProcess(command, 0, "fixture converter", "")

    def test_common_cook_preserves_uv1_uv2_color_and_tangent_with_real_contract(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary); asset, gltf, converter = self.geometry_fixture(root)
            with patch.object(pipeline, "run", side_effect=self.converter_stub):
                result = pipeline.cook_one(asset, root, converter, 10, False)
            model = root / "runtime/FIXTURE/FIXTURE.wmodel"
            decoded = geometry.parse_geometry_wmodel(model.read_bytes())
            self.assertTrue(decoded["hasColor0"])
            self.assertTrue(decoded["hasTexcoord1"])
            self.assertTrue(decoded["hasTexcoord2"])
            geometry.verify_source_against_geometry_contract(gltf, model)
            contract = json.loads((model.parent / "geometry.receipt.json").read_text())
            self.assertFalse(contract["runtimeProductAdmission"])
            self.assertEqual(contract["provenanceEvidence"]["sourcePackage"]["status"], "OBSERVED_UNBOUND")
            self.assertEqual(contract["provenanceEvidence"]["sourceManifest"]["hashRole"],
                             "OBSERVED_GENERATED_COOK_INPUTS_CANONICAL_LF")
            self.assertEqual(result["receipt"]["schemaVersion"], 2)
            with patch.object(pipeline, "run", side_effect=AssertionError("valid cook should resume")):
                self.assertEqual(pipeline.cook_one(asset, root, converter, 10, False)["status"], "resumed")

    def test_common_cook_does_not_create_absent_optional_channels(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary); asset, gltf, converter = self.geometry_fixture(root, optional=False)
            with patch.object(pipeline, "run", side_effect=self.converter_stub):
                pipeline.cook_one(asset, root, converter, 10, False)
            decoded = geometry.parse_geometry_wmodel((root / "runtime/FIXTURE/FIXTURE.wmodel").read_bytes())
            self.assertFalse(decoded["hasColor0"])
            self.assertFalse(decoded["hasTexcoord1"])
            self.assertFalse(decoded["hasTexcoord2"])

    def test_tampered_buffer_rejects_before_converter_and_preserves_old_pack(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary); asset, gltf, converter = self.geometry_fixture(root)
            destination = root / "runtime/FIXTURE"; destination.mkdir(parents=True)
            (destination / "sentinel").write_bytes(b"previous")
            (gltf.parent / "fixture.bin").write_bytes(b"changed")
            with patch.object(pipeline, "run", side_effect=AssertionError("must reject before converter")):
                with self.assertRaisesRegex(pipeline.PipelineError, "hash mismatch"):
                    pipeline.cook_one(asset, root, converter, 10, True)
            self.assertEqual((destination / "sentinel").read_bytes(), b"previous")

    def test_missing_source_tangent_rejects_without_synthesis_or_commit(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary); asset, gltf, converter = self.geometry_fixture(root)
            document = json.loads(gltf.read_text())
            del document["meshes"][0]["primitives"][0]["attributes"]["TANGENT"]
            gltf.write_text(json.dumps(document))
            self.refresh_source_receipt(gltf.parent, asset, root / "fixture.upk")
            with patch.object(pipeline, "run", side_effect=self.converter_stub):
                with self.assertRaisesRegex(pipeline.PipelineError, "required indexed geometry channel"):
                    pipeline.cook_one(asset, root, converter, 10, False)
            self.assertFalse((root / "runtime/FIXTURE").exists())

    def test_missing_package_evidence_rejects_without_fake_hash(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary); asset, gltf, converter = self.geometry_fixture(root)
            (root / "fixture.upk").unlink()
            with patch.object(pipeline, "run", side_effect=self.converter_stub):
                with self.assertRaisesRegex(pipeline.PipelineError, "recorded physical package"):
                    pipeline.cook_one(asset, root, converter, 10, False)
            self.assertFalse((root / "runtime/FIXTURE").exists())

    def test_converter_revision_invalidates_runtime_resume(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary); asset, gltf, converter = self.geometry_fixture(root)
            with patch.object(pipeline, "run", side_effect=self.converter_stub):
                pipeline.cook_one(asset, root, converter, 10, False)
            converter.write_bytes(b"different converter identity")
            with patch.object(pipeline, "run", side_effect=self.converter_stub) as run:
                self.assertEqual(pipeline.cook_one(asset, root, converter, 10, False)["status"], "cooked")
                self.assertEqual(run.call_count, 2)

    def test_material_slot_rename_keeps_original_geometry_evidence(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary); asset, gltf, converter = self.geometry_fixture(root)
            stage = root / "stage"; stage.mkdir()
            renamed = stage / gltf.name
            document = json.loads(gltf.read_text()); document["materials"][0]["name"] = "SLOT_000_fixture"
            renamed.write_text(json.dumps(document))
            shutil.copy2(gltf.parent / "fixture.bin", stage / "fixture.bin")
            model = _build_legacy_wmodel(stage / "variant.wmodel")
            result = pipeline.preserve_cooked_geometry(source_gltf=renamed, model=model,
                source_receipt_path=gltf.parent / "source.receipt.json", converter=converter,
                command=[str(converter), str(renamed)], source_object=asset["fullPath"])
            self.assertEqual(result["sourceGltfSha256"], pipeline.sha256(renamed).lower())
            geometry.verify_source_against_geometry_contract(renamed, model)

    def test_topology_mismatch_keeps_previous_runtime_pack(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary); asset, gltf, converter = self.geometry_fixture(root)
            destination = root / "runtime/FIXTURE"; destination.mkdir(parents=True)
            (destination / "sentinel").write_bytes(b"previous")
            document = json.loads(gltf.read_text())
            indices = document["accessors"][document["meshes"][0]["primitives"][0]["indices"]]
            view = document["bufferViews"][indices["bufferView"]]
            buffer = gltf.parent / "fixture.bin"; payload = bytearray(buffer.read_bytes())
            struct.pack_into("<3H", payload, view["byteOffset"], 0, 2, 1)
            buffer.write_bytes(payload)
            self.refresh_source_receipt(gltf.parent, asset, root / "fixture.upk")
            with patch.object(pipeline, "run", side_effect=self.converter_stub):
                with self.assertRaisesRegex(pipeline.PipelineError, "topology differ"):
                    pipeline.cook_one(asset, root, converter, 10, True)
            self.assertEqual((destination / "sentinel").read_bytes(), b"previous")

    def test_runtime_manifest_refuses_legacy_geometry_receipt(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary); asset, gltf, converter = self.geometry_fixture(root)
            with patch.object(pipeline, "run", side_effect=self.converter_stub):
                pipeline.cook_one(asset, root, converter, 10, False)
            inventory = {"areaId": "FIXTURE", "assets": [asset]}
            pipeline.build_runtime_manifest(root, inventory, 1)
            receipt_path = root / "runtime/FIXTURE/runtime.receipt.json"
            receipt = json.loads(receipt_path.read_text()); receipt["schemaVersion"] = 1
            pipeline.atomic_write_json(receipt_path, receipt)
            with self.assertRaisesRegex(pipeline.PipelineError, "stale or incomplete"):
                pipeline.build_runtime_manifest(root, inventory, 1)

    def test_inventory_deduplicates_assets_and_filters_level_prefix(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            document = {
                "schemaVersion": 1,
                "propertyErrors": [],
                "placements": [
                    {
                        "placementId": "main:1",
                        "levelPackage": "LV_BER_BERNCASTLE_T_SL00",
                        "asset": {
                            "objectPath": "bg_test_a.mesh.bg_test_box_sm",
                            "objectName": "bg_test_box_sm",
                        },
                    },
                    {
                        "placementId": "main:2",
                        "levelPackage": "LV_BER_BERNCASTLE_T_SL00",
                        "asset": {
                            "objectPath": "BG_TEST_A.MESH.BG_TEST_BOX_SM",
                            "objectName": "BG_TEST_BOX_SM",
                        },
                    },
                    {
                        "placementId": "fav:1",
                        "levelPackage": "LV_BER_BERNCASTLE_FAV_PS",
                        "asset": {
                            "objectPath": "bg_fav.mesh.bg_fav_sm",
                            "objectName": "bg_fav_sm",
                        },
                    },
                ],
            }
            path = root / "source.placements.json"
            path.write_text(json.dumps(document), encoding="utf-8")
            result = pipeline.inventory_assets(
                [root], "LV_BER_BERNCASTLE_T_", 1, 2
            )
            self.assertEqual(result["assetCount"], 1)
            self.assertEqual(result["placementCount"], 2)
            self.assertEqual(
                result["assets"][0]["fullPath"],
                "bg_test_a.mesh.bg_test_box_sm",
            )

    def visibility_source(self, *, hidden=False):
        from extract_ue3_placements import source_visibility_from_chains
        actor = [{"objectPath": "Actor", "physicalPackage": "source.upk",
                  "packageSha256": "a" * 64, "exportIndex": 10,
                  "serializedFlags": {"bHidden": hidden}}]
        component = [{"objectPath": "Actor.Component", "physicalPackage": "source.upk",
                      "packageSha256": "a" * 64, "exportIndex": 11,
                      "serializedFlags": {}}]
        return source_visibility_from_chains(actor, component)

    def test_inventory_v3_keeps_visible_helper_names_and_hidden_assets(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            # Initialize the shared parser through the production lazy import.
            path = root / "source.placements.json"
            path.write_text(json.dumps({"schemaVersion": 1, "placements": []}))
            pipeline.inventory_assets([root], "", 0, 0)
            rows = []
            for index, (name, hidden) in enumerate((("lv_module", False),
                                                   ("bfx_cloudplane", False),
                                                   ("ordinary_floor", True))):
                rows.append({"placementId": f"LV_TEST:{index}", "levelPackage": "LV_TEST",
                             "asset": {"objectPath": f"fixture.mesh.{name}", "objectName": name},
                             "sourceVisibility": self.visibility_source(hidden=hidden)})
            document = {"schemaVersion": 3, "propertyErrors": [], "placements": rows}
            path.write_text(json.dumps(document), encoding="utf-8")
            before = path.read_bytes()
            result = pipeline.inventory_assets([root], "LV_TEST", 3, 3)
            self.assertEqual(result["sourceVisibilityCounts"], {"hidden": 1, "visible": 2})
            self.assertEqual(result["sourceVisibilityBasisCounts"], {"ue3-instance-archetype-cdo": 3})
            self.assertEqual(result["sources"][0]["sourceVisibilityCounts"], result["sourceVisibilityCounts"])
            self.assertEqual(result["sources"][0]["schemaVersion"], 3)
            self.assertEqual(result["sources"][0]["sha256"], pipeline.sha256(path))
            self.assertEqual(path.read_bytes(), before)
            self.assertEqual({row["objectName"] for row in result["assets"]},
                             {"lv_module", "bfx_cloudplane", "ordinary_floor"})

    def test_inventory_v2_keeps_unrecorded_visibility_explicit(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            document = {"schemaVersion": 2, "placements": [{"levelPackage": "LV_TEST",
                        "asset": {"objectPath": "fixture.mesh.floor", "objectName": "floor"}}]}
            (root / "source.placements.json").write_text(json.dumps(document))
            result = pipeline.inventory_assets([root], "", 1, 1)
            self.assertEqual(result["sourceVisibilityCounts"], {"unrecorded": 1})
            self.assertEqual(result["sourceVisibilityBasisCounts"], {"legacy-unrecorded": 1})

    def test_inventory_rejects_invalid_visibility_and_legacy_evidence(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            path = root / "source.placements.json"
            path.write_text(json.dumps({"schemaVersion": 1, "placements": []}))
            pipeline.inventory_assets([root], "", 0, 0)
            valid = self.visibility_source()
            forged = json.loads(json.dumps(valid))
            forged["visible"] = False
            empty_chain = json.loads(json.dumps(valid))
            empty_chain["actorChain"] = []
            bad_hash = json.loads(json.dumps(valid))
            bad_hash["componentChain"][0]["packageSha256"] = "missing"
            for schema, visibility in ((3, None), (3, forged), (3, empty_chain),
                                        (3, bad_hash), (1, valid), (2, valid)):
                with self.subTest(schema=schema, visibility=visibility):
                    row = {"levelPackage": "LV_TEST", "asset": {
                        "objectPath": "fixture.mesh.floor", "objectName": "floor"}}
                    if visibility is not None:
                        row["sourceVisibility"] = visibility
                    path.write_text(json.dumps({"schemaVersion": schema, "placements": [row]}))
                    with self.assertRaisesRegex(pipeline.PipelineError, "invalid placement visibility"):
                        pipeline.inventory_assets([root], "", None, None)

    def test_parse_material_props_reads_texture_parameter(self):
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "sample.props.txt"
            path.write_text(
                "Parent = MaterialInstanceConstant'base.parent_mi'\n"
                "TextureParameterValues[1] =\n{\n"
                " ParameterValue = Texture2D'tex.sample_d'\n"
                " ParameterName = texture_diffuse\n}\n",
                encoding="utf-8",
            )
            parent, values = pipeline.parse_material_props(path)
            self.assertEqual(parent, "base.parent_mi")
            self.assertEqual(values["texture_diffuse"], "sample_d")

    def test_vertex_blend_texture_roles_prefer_red_channel(self):
        selected = pipeline.select_supported_texture_parameters(
            {
                "a_texture_diffuse": "grass_d",
                "r_texture_diffuse": "bark_d",
                "r_texture_normal": "bark_n",
            }
        )
        self.assertEqual(
            selected["--material-remap"],
            ("r_texture_diffuse", "bark_d"),
        )
        self.assertEqual(
            selected["--normal-remap"],
            ("r_texture_normal", "bark_n"),
        )

    def test_regular_texture_role_wins_over_vertex_blend_fallback(self):
        selected = pipeline.select_supported_texture_parameters(
            {
                "texture_diffuse": "authored_d",
                "r_texture_diffuse": "fallback_d",
            }
        )
        self.assertEqual(
            selected["--material-remap"],
            ("texture_diffuse", "authored_d"),
        )

    def test_legacy_layer_texture_roles_are_supported(self):
        selected = pipeline.select_supported_texture_parameters(
            {
                "layer01_diffuse": "mountain_d",
                "normalmap": "mountain_n",
            }
        )
        self.assertEqual(
            selected["--material-remap"],
            ("layer01_diffuse", "mountain_d"),
        )
        self.assertEqual(
            selected["--normal-remap"],
            ("normalmap", "mountain_n"),
        )


if __name__ == "__main__":
    unittest.main()
