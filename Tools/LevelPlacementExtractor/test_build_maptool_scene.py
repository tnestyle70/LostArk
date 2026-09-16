import argparse
import hashlib
import json
import math
import tempfile
import struct
import unittest
from pathlib import Path
from unittest.mock import patch

from build_maptool_scene import (
    EMPTY_MATERIAL_SIGNATURE,
    IMPORTED_ID_BIT,
    classify_asset_name_hint,
    compile_scene,
    convert_position,
    convert_rotation,
    convert_scale,
    directx_row_matrix_from_quaternion,
    imported_id,
    material_signature_from_slots,
    parse_args,
    scale_flags,
    source_visibility_from_chains,
    validate_source_material_geometry,
)


class MapToolSceneTransformTests(unittest.TestCase):
    def assert_vector_close(self, actual, expected, places=6):
        self.assertEqual(len(actual), len(expected))
        for left, right in zip(actual, expected):
            self.assertAlmostEqual(left, right, places=places)

    def test_position_basis_and_centimeter_to_meter(self):
        actual = convert_position({"x": 849.4918212890625, "y": 2140.9453125, "z": 975.8194580078125})
        self.assert_vector_close(actual, (8.494918212890625, 9.758194580078125, -21.409453125))

    def test_scale_reorders_axes_and_preserves_sign(self):
        actual = convert_scale({"x": -2.0, "y": 3.0, "z": 4.0})
        self.assertEqual(actual, (-2.0, 4.0, 3.0))

    def test_two_negative_axes_are_not_reflection(self):
        self.assertEqual(scale_flags((-2.0, -3.0, 4.0)), (True, False))

    def test_one_negative_axis_is_reflection(self):
        self.assertEqual(scale_flags((-2.0, 3.0, 4.0)), (True, True))

    def test_identity_rotation(self):
        actual = convert_rotation({"pitch": 0, "yaw": 0, "roll": 0})
        self.assert_vector_close(actual, (0.0, 0.0, 0.0, 1.0))

    def test_central_floor_yaw(self):
        actual = convert_rotation({"pitch": 0, "yaw": -16384, "roll": 0})
        root = math.sqrt(0.5)
        self.assert_vector_close(actual, (0.0, -root, 0.0, root))
        expected_rows = ((0.0, 0.0, 1.0), (0.0, 1.0, 0.0), (-1.0, 0.0, 0.0))
        rows = directx_row_matrix_from_quaternion(actual)
        for row, expected in zip(rows, expected_rows):
            self.assert_vector_close(row, expected)

    def test_imported_id_is_stable_and_uses_high_bit(self):
        source = "LV_LUT_HEARTRB_ED_SL00:export:1234"
        self.assertEqual(imported_id(source), 11534871182138487613)
        self.assertNotEqual(imported_id(source) & IMPORTED_ID_BIT, 0)
        self.assertEqual(imported_id(source), imported_id(source))

    def test_lv_navimesh_name_is_diagnostic_only(self):
        asset = {
            "rootImport": "lv_navimesh",
            "logicalPackage": "LV_NAVIMESH",
            "fullPath": "lv_navimesh.mesh.lv_common_mesh_cul_box_8",
        }
        self.assertEqual(classify_asset_name_hint(asset), "nav-package")

    def test_lv_module_name_is_diagnostic_only(self):
        asset = {
            "rootImport": "lv_module",
            "logicalPackage": "LV_MODULE",
            "objectName": "lv_module_water02_512",
            "fullPath": "lv_module.mesh.lv_module_water02_512",
        }
        self.assertEqual(classify_asset_name_hint(asset), "module-package")

    def test_water_name_is_diagnostic_only(self):
        asset = {
            "rootImport": "lv_lut_heartrb",
            "logicalPackage": "LV_LUT_HEARTRB",
            "objectName": "lv_lut_heartrb_water01_sm",
            "fullPath": "lv_lut_heartrb.mesh.lv_lut_heartrb_water01_sm",
        }
        self.assertEqual(classify_asset_name_hint(asset), "water-name")

    def test_bfx_name_is_diagnostic_only(self):
        asset = {
            "rootImport": "bfx_sm_00",
            "logicalPackage": "BFX_SM_00",
            "objectName": "bfm_mossfog_001",
            "fullPath": "bfx_sm_00.bfm_mossfog_001",
        }
        self.assertEqual(classify_asset_name_hint(asset), "fx-name")

    def test_regular_level_asset_remains_visible(self):
        asset = {
            "rootImport": "pvp_retown_a",
            "logicalPackage": "PVP_RETOWN_A",
            "fullPath": "pvp_retown_a.mesh.bg_pvp_retown_floor01_sm",
        }
        self.assertIsNone(classify_asset_name_hint(asset))


class SourceMaterialGeometryChannelTests(unittest.TestCase):
    def test_rnm_and_overlay_require_actual_uv1_and_color_bytes(self):
        with tempfile.TemporaryDirectory() as temporary:
            model = Path(temporary) / "mesh.wmodel"
            model.write_bytes(b"actual model payload")
            rows = [{"bakedLighting": {"averageTexture": "average.dds"}},
                    {"family": "bg_base_opa_overlay", "sourceOverlayFlags": 4}]
            parsed = {"hasTexcoord1": True, "hasColor0": True, "vertexStride": 60, "submeshes": [{}]}
            with patch("build_maptool_scene.geometry.parse_geometry_wmodel", return_value=parsed) as parse:
                result = validate_source_material_geometry(model, rows)
                parse.assert_called_once_with(b"actual model payload")
                self.assertEqual(result["requiredChannels"], ["COLOR0", "TEXCOORD1"])
                self.assertEqual(result["modelSha256"], hashlib.sha256(model.read_bytes()).hexdigest())
            for field, channel in (("hasTexcoord1", "TEXCOORD1"), ("hasColor0", "COLOR0")):
                with self.subTest(field=field), patch("build_maptool_scene.geometry.parse_geometry_wmodel", return_value={**parsed, field: False}):
                    with self.assertRaisesRegex(ValueError, channel):
                        validate_source_material_geometry(model, rows)

    def test_legacy_without_rnm_or_vertex_overlay_keeps_existing_path(self):
        with patch("build_maptool_scene.geometry.parse_geometry_wmodel") as parse:
            result = validate_source_material_geometry(Path("unused.wmodel"), [
                {"family": "bg_base_pbr_opa"}, {"family": "bg_base_opa_overlay", "sourceOverlayFlags": 0}])
            parse.assert_not_called()
            self.assertEqual(result["validation"], "no-extra-channel-required")

    def test_malformed_actual_model_cannot_supply_required_channels(self):
        with tempfile.TemporaryDirectory() as temporary:
            model = Path(temporary) / "mesh.wmodel"
            model.write_bytes(b"WMOD-four-byte-magic-is-not-geometry")
            for row in ({"bakedLighting": {"averageTexture": "average.dds"}}, {"family": "bg_base_opa_overlay"}):
                with self.assertRaisesRegex(ValueError, "cannot be verified"):
                    validate_source_material_geometry(model, [row])


class MapToolSceneCompileTests(unittest.TestCase):
    @staticmethod
    def write_json(path: Path, value) -> None:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(value), encoding="utf-8")

    @staticmethod
    def asset(
        asset_id: str,
        *,
        full_path: str | None = None,
        material_signature: str | None = None,
    ) -> dict:
        result = {
            "assetId": asset_id,
            "objectName": asset_id,
            "fullPath": full_path or f"test.mesh.{asset_id}",
            "sourceCategory": "StaticMesh",
            "logicalPackage": "TEST",
            "rootImport": "TEST",
        }
        if material_signature is not None:
            result["materialSignatureSha256"] = material_signature
        return result

    @staticmethod
    def placement(source_id: str, asset_id: str, level: str) -> dict:
        return {
            "placementId": source_id,
            "levelPackage": level,
            "asset": {"objectPath": f"test.mesh.{asset_id}"},
            "transform": {
                "source": "actor",
                "position": {"x": 0.0, "y": 0.0, "z": 0.0},
                "rotation": {"pitch": 0, "yaw": 0, "roll": 0},
                "scale3D": {"x": 1.0, "y": 1.0, "z": 1.0},
            },
        }

    @staticmethod
    def arguments(
        root: Path,
        asset_manifest: Path,
        runtime_manifest: Path,
        runtime_root: Path,
        placements_dir,
        *,
        include_source_id=None,
        include_level=None,
        suffix="",
    ) -> argparse.Namespace:
        return argparse.Namespace(
            area_id="TEST",
            asset_manifest=asset_manifest,
            runtime_manifest=runtime_manifest,
            runtime_root=runtime_root,
            runtime_asset_root=None,
            overlay_manifest=None,
            render_profile_manifest=None,
            allow_legacy_visibility=True,
            allow_legacy_material_coverage=True,
            allow_partial_material_preview=False,
            placements_dir=placements_dir,
            catalog_output=root / f"catalog{suffix}.txt",
            placement_output=root / f"placements{suffix}.txt",
            receipt_output=root / f"receipt{suffix}.json",
            golden_placement_id="",
            include_source_id=[] if include_source_id is None else include_source_id,
            include_level=[] if include_level is None else include_level,
            expect_assets=None,
            expect_output_assets=None,
            expect_source_placements=None,
            expect_output_placements=None,
            expect_output_any_negative=None,
            expect_output_reflected=None,
            expect_any_negative=None,
            expect_reflected=None,
            expect_hidden_helpers=None,
            expect_output_hidden_helpers=None,
            expect_overlay_assets=None,
            expect_overlay_placements=None,
            expect_hidden_category=[],
            expect_level_count=[],
        )

    def visibility_fixture(self, root, *, hidden=False, name="LV_MODULE.Mesh.OpaqueFloor"):
        actor = [{"objectPath": "Actor", "physicalPackage": "source.upk", "packageSha256": "a" * 64,
                  "exportIndex": 280, "serializedFlags": {"bHidden": hidden}}]
        component = [{"objectPath": "Actor.Component", "physicalPackage": "source.upk", "packageSha256": "a" * 64,
                      "exportIndex": 488, "serializedFlags": {}}]
        placement = self.placement("LV_TEST:export:488", "floor", "LV_TEST")
        placement["asset"]["objectPath"] = name
        placement["materialOverrides"] = {"propertyPresent": False, "slots": [], "signatureSha256": EMPTY_MATERIAL_SIGNATURE}
        placement["sourceVisibility"] = source_visibility_from_chains(actor, component)
        asset = self.asset("floor", full_path=name)
        asset["logicalPackage"] = asset["rootImport"] = name.split(".")[0]
        asset["objectName"] = name.split(".")[-1]
        self.write_json(root / "assets.json", {"areaId": "TEST", "assets": [asset]})
        runtime = {"assetId": "floor", "model": "floor.wmodel", "runtimeCoverage": {
            "textureDependencyClosureComplete": True, "textureSlotsComplete": True, "materialComplete": True},
            "sourceOnlyUnsupported": []}
        self.write_json(root / "runtime.json", {"areaId": "TEST", "assets": [runtime]})
        self.write_json(root / "placements/source.placements.json", {"schemaVersion": 3, "placements": [placement]})
        (root / "floor.wmodel").write_bytes(b"WMOD-fixture")
        args = self.arguments(root, root / "assets.json", root / "runtime.json", root, root / "placements")
        args.allow_legacy_visibility = args.allow_legacy_material_coverage = False
        return args, placement, runtime

    def test_source_visible_module_nav_water_and_fx_names_are_not_hidden(self):
        for name in ("LV_MODULE.Mesh.OpaqueFloor", "LV_NAVIMESH.Mesh.Box", "LV_LUT_HEARTRB.Mesh.lv_lut_heartrb_water01", "BFX_SM_00.bfm_cloudplane"):
            with self.subTest(name=name), tempfile.TemporaryDirectory() as temporary:
                args, _, _ = self.visibility_fixture(Path(temporary), name=name)
                receipt = compile_scene(args)
                self.assertTrue(args.placement_output.read_text().splitlines()[1].endswith(" 1"))
                self.assertEqual(receipt["sourceHiddenCategoryCounts"], {})
                self.assertEqual(sum(receipt["assetNameHintsDiagnosticOnly"].values()), 1)

    def test_original_hidden_flag_and_explicit_override_are_separate(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            args, _, _ = self.visibility_fixture(root, hidden=True)
            self.assertEqual(compile_scene(args)["hiddenCategoryCounts"], {"source-hidden": 1})
            args.render_profile_manifest = root / "profile.json"
            self.write_json(args.render_profile_manifest, {"schemaVersion": 1, "areaId": "TEST", "profiles": [],
                "visibilityOverrides": [{"sourcePlacementId": "LV_TEST:export:488", "visible": True}]})
            self.assertEqual(compile_scene(args)["hiddenCategoryCounts"], {})

    def test_visibility_tamper_and_unknown_legacy_fail_without_replacing_outputs(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            args, placement, _ = self.visibility_fixture(root)
            compile_scene(args)
            previous = args.placement_output.read_bytes()
            placement["sourceVisibility"]["visible"] = False
            self.write_json(root / "placements/source.placements.json", {"schemaVersion": 3, "placements": [placement]})
            with self.assertRaisesRegex(ValueError, "visible mismatch"):
                compile_scene(args)
            self.assertEqual(previous, args.placement_output.read_bytes())
            del placement["sourceVisibility"]
            self.write_json(root / "placements/source.placements.json", {"schemaVersion": 2, "placements": [placement]})
            with self.assertRaisesRegex(ValueError, "visibility was not extracted"):
                compile_scene(args)
            args.allow_legacy_visibility = True
            self.assertEqual(compile_scene(args)["sourceVisibilityBasisCounts"], {"legacy-unrecorded": 1})

    def test_material_support_failure_is_not_a_visibility_override(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            args, _, runtime = self.visibility_fixture(root)
            compile_scene(args)
            previous = args.placement_output.read_bytes()
            runtime["runtimeCoverage"]["materialComplete"] = False
            runtime["sourceOnlyUnsupported"] = [{"material": "NativeWater", "unsupported": ["shader:water"]}]
            self.write_json(root / "runtime.json", {"areaId": "TEST", "assets": [runtime]})
            with self.assertRaisesRegex(ValueError, "sourceOnlyUnsupported"):
                compile_scene(args)
            self.assertEqual(previous, args.placement_output.read_bytes())
            args.allow_partial_material_preview = True
            receipt = compile_scene(args)
            self.assertEqual(receipt["runtimeMaterialAdmission"]["floor"]["mode"], "geometry-preview-partial-material")
            self.assertTrue(args.placement_output.read_text().splitlines()[1].endswith(" 1"))
            runtime["runtimeCoverage"]["textureDependencyClosureComplete"] = False
            self.write_json(root / "runtime.json", {"areaId": "TEST", "assets": [runtime]})
            with self.assertRaisesRegex(ValueError, "texture dependency closure"):
                compile_scene(args)

    def source_material_fixture(self, root):
        args, placement, runtime = self.visibility_fixture(root)
        args.runtime_asset_root = root / "Resources"
        texture = args.runtime_asset_root / "Map/source.dds"
        texture.parent.mkdir(parents=True)
        header = [0] * 31
        header[0], header[2], header[3], header[6] = 124, 4, 4, 1
        header[18], header[19], header[20], header[26] = 32, 4, int.from_bytes(b"DXT1", "little"), 0x1000
        texture.write_bytes(b"DDS " + struct.pack("<31I", *header) + b"x" * 8)
        runtime["runtimeCoverage"]["materialComplete"] = False
        runtime["sourceOnlyUnsupported"] = [{"slot": 0, "fields": ["scalar:diffuse_brightness", "texture:texture_reflection"]}]
        runtime["materials"] = [{"slot": 0, "runtimeName": "FloorSlot", "objectPath": "PKG.Material.Floor", "sourceOnlyReason": None}]
        self.write_json(root / "runtime.json", {"areaId": "TEST", "assets": [runtime]})
        import build_source_map_materials as builder
        values = {key: 1.0 for key in (
            "diffuse_brightness", "normal_intensity", "reflection_contrast", "metallic_intensity", "metallic_power",
            "roughness_intensity", "roughness_power", "ao_intensity", "ao_power", "specular_pbr_intensity",
            "nonmetallic_brightness", "metallic_brightness")}
        values.update(diffuse_brightness=0.0, diffuse_color=[1, 1, 1, 1], reflection_color=[1, 1, 1, 1])
        parameters = {"format": "lostark-source-map-material-parameters", "formatVersion": 1, "failures": [], "sources": [],
            "materials": {"pkg.material.floor": {"sourceMaterial": "pkg.material.floor", "terminal": "original.graph.bg_base_pbr_opa",
            "values": values, "switches": {"1.use_diffuse_to_albedo": True, "1.use_normalmap": True},
            "textures": {key: "source.texture.dds" for key in ("texture_diffuse", "texture_normal", "texture_detail_normal", "texture_orm", "texture_reflection")}}}}
        self.write_json(root / "native-parameters.json", parameters)
        (root / "native-evidence.json").write_text("{}")
        def file_record(path):
            return {"path": str(path), "sha256": hashlib.sha256(path.read_bytes()).hexdigest()}
        manifest = {"format": "lostark-source-map-material-input", "formatVersion": 1, "areaId": "TEST",
            "parameters": file_record(root / "native-parameters.json"), "evidence": [file_record(root / "native-evidence.json")],
            "textures": [{"sourceObject": "source.texture.dds", "assetId": "Map/source.dds", "sha256": hashlib.sha256(texture.read_bytes()).hexdigest(),
                          "colorSpace": "linear", "mipCount": 1, "mipEvidence": "source-unmipped"}],
            "slots": [{"assetId": "floor", "materialName": "FloorSlot", "sourceMaterial": "pkg.material.floor", "component": {
                "rendering": {"castsShadow": True, "renderMode": "deferred", "cullMode": "back"},
                "lightingEvidence": "source-absent", "environmentEvidence": "source-absent",
                "minimumRoughness": 0.04, "minimumRoughnessEvidence": "project-authored"}}]}
        self.write_json(root / "native-input.json", manifest)
        output, receipt = builder.compile_materials(root / "native-input.json", args.runtime_asset_root)
        self.write_json(root / "native-materials.json", output)
        receipt["output"] = file_record(root / "native-materials.json")
        args.source_materials_receipt = root / "native-receipt.json"
        args.materials_output = root / "authoring-materials.json"
        self.write_json(args.source_materials_receipt, receipt)
        return args, runtime, receipt

    def test_verified_native_materials_cover_known_legacy_gaps_and_preserve_zero(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            args, runtime, native_receipt = self.source_material_fixture(root)
            receipt = compile_scene(args)
            self.assertEqual(receipt["runtimeMaterialAdmission"]["floor"]["mode"], "source-material-inputs-complete")
            self.assertEqual(json.loads(args.materials_output.read_text())["materials"][0]["diffuseBrightness"], 0)
            self.assertFalse(receipt["sourceMaterialBuild"]["originalVisualFidelityVerified"])
            runtime["sourceOnlyUnsupported"][0]["fields"].append("material:unknown-source-carrier")
            self.write_json(root / "runtime.json", {"areaId": "TEST", "assets": [runtime]})
            previous = args.materials_output.read_bytes()
            with self.assertRaisesRegex(ValueError, "coverage incomplete"):
                compile_scene(args)
            self.assertEqual(previous, args.materials_output.read_bytes())

    def test_adding_unknown_scalar_to_receipt_does_not_claim_mapper_support(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            args, runtime, native_receipt = self.source_material_fixture(root)
            compile_scene(args)
            previous = args.materials_output.read_bytes()
            runtime["sourceOnlyUnsupported"][0]["fields"].append("scalar:unknown_future_parameter")
            self.write_json(root / "runtime.json", {"areaId": "TEST", "assets": [runtime]})
            with self.assertRaisesRegex(ValueError, "coverage incomplete"):
                compile_scene(args)
            native_receipt["bindings"][0]["materialCoverage"][0]["coveredSourceOnly"].append("scalar:unknown_future_parameter")
            self.write_json(args.source_materials_receipt, native_receipt)
            with self.assertRaisesRegex(ValueError, "pinned compiler inputs"):
                compile_scene(args)
            self.assertEqual(previous, args.materials_output.read_bytes())

    def test_scene_outputs_cannot_overwrite_pinned_source_evidence(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            args, _, _ = self.source_material_fixture(root)
            evidence = root / "native-parameters.json"
            before = evidence.read_bytes()
            args.materials_output = evidence
            with self.assertRaisesRegex(ValueError, "overwrite source inputs"):
                compile_scene(args)
            self.assertEqual(before, evidence.read_bytes())
            self.assertFalse(args.catalog_output.exists())

    def test_rnm_material_cannot_commit_with_a_model_without_verified_uv1(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            args, _, _ = self.source_material_fixture(root)
            compile_scene(args)
            targets = (args.catalog_output, args.placement_output, args.materials_output, args.receipt_output)
            previous = [path.read_bytes() for path in targets]
            manifest_path = root / "native-input.json"
            manifest = json.loads(manifest_path.read_text())
            manifest["slots"][0]["component"].update(lightingEvidence="source-bound", bakedLighting={
                "averageTexture": "Map/source.dds", "directionalTexture": "Map/source.dds", "colorSpace": "linear"})
            self.write_json(manifest_path, manifest)
            import build_source_map_materials as builder
            output, receipt = builder.compile_materials(manifest_path, args.runtime_asset_root)
            self.write_json(root / "native-materials.json", output)
            receipt["output"] = {"path": str(root / "native-materials.json"), "sha256": hashlib.sha256((root / "native-materials.json").read_bytes()).hexdigest()}
            self.write_json(args.source_materials_receipt, receipt)
            with self.assertRaisesRegex(ValueError, "geometry channels cannot be verified"):
                compile_scene(args)
            self.assertEqual(previous, [path.read_bytes() for path in targets])

    def test_source_material_evidence_and_slot_mismatch_fail_before_outputs_change(self):
        for field in ("input", "output", "resource", "slot"):
            with self.subTest(field=field), tempfile.TemporaryDirectory() as temporary:
                root = Path(temporary)
                args, runtime, receipt = self.source_material_fixture(root)
                compile_scene(args)
                old = [path.read_bytes() for path in (args.catalog_output, args.placement_output, args.materials_output)]
                if field == "input":
                    (root / "native-input.json").write_text("changed")
                elif field == "output":
                    (root / "native-materials.json").write_text("changed")
                elif field == "resource":
                    (args.runtime_asset_root / "Map/source.dds").write_bytes(b"changed")
                else:
                    runtime["materials"][0]["objectPath"] = "PKG.Material.Other"
                    self.write_json(root / "runtime.json", {"areaId": "TEST", "assets": [runtime]})
                with self.assertRaises(ValueError):
                    compile_scene(args)
                self.assertEqual(old, [path.read_bytes() for path in (args.catalog_output, args.placement_output, args.materials_output)])

    def test_scene_promotion_failure_rolls_back_catalog_placements_materials_and_receipt(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            args, _, _ = self.source_material_fixture(root)
            compile_scene(args)
            targets = (args.catalog_output, args.placement_output, args.materials_output, args.receipt_output)
            old = [path.read_bytes() for path in targets]
            import os
            replace = os.replace
            count = 0
            def fail_second(source, target):
                nonlocal count
                count += 1
                if count == 2:
                    raise OSError("promotion fixture")
                replace(source, target)
            with patch("build_maptool_scene.os.replace", side_effect=fail_second), self.assertRaisesRegex(OSError, "promotion fixture"):
                compile_scene(args)
            self.assertEqual(old, [path.read_bytes() for path in targets])

    def test_repeatable_directories_and_level_filter_prune_exact_assets(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            asset_manifest = root / "assets.json"
            runtime_manifest = root / "runtime.json"
            runtime_root = root / "runtime"
            first_directory = root / "first"
            second_directory = root / "second"
            runtime_root.mkdir()

            assets = [self.asset("asset_a"), self.asset("asset_b"), self.asset("asset_c")]
            self.write_json(asset_manifest, {"areaId": "TEST", "assets": assets})
            self.write_json(
                runtime_manifest,
                {
                    "areaId": "TEST",
                    "assets": [
                        {"assetId": asset["assetId"], "model": f"{asset['assetId']}.wmodel"}
                        for asset in assets
                    ],
                },
            )
            (runtime_root / "asset_a.wmodel").write_bytes(b"WMOD-a")
            (runtime_root / "asset_b.wmodel").write_bytes(b"WMOD-b")
            self.write_json(
                first_directory / "shared.placements.json",
                {
                    "schemaVersion": 1,
                    "propertyErrors": [],
                    "placements": [self.placement("source_a", "asset_a", "LEVEL_A")],
                },
            )
            self.write_json(
                second_directory / "shared.placements.json",
                {
                    "schemaVersion": 1,
                    "propertyErrors": [],
                    "placements": [self.placement("source_b", "asset_b", "LEVEL_B")],
                },
            )

            arguments = self.arguments(
                root,
                asset_manifest,
                runtime_manifest,
                runtime_root,
                [first_directory, second_directory],
                include_level=["LEVEL_A"],
            )
            arguments.expect_assets = 3
            arguments.expect_output_assets = 1
            arguments.expect_source_placements = 2
            arguments.expect_output_placements = 1
            receipt = compile_scene(arguments)

            catalog = arguments.catalog_output.read_text(encoding="utf-8")
            self.assertIn('LOSTARK_MAP_ASSET_CATALOG 4 "TEST" 1', catalog)
            self.assertIn('"asset_a"', catalog)
            self.assertNotIn('"asset_b"', catalog)
            self.assertNotIn('"asset_c"', catalog)
            self.assertEqual(receipt["exactAssetCount"], 3)
            self.assertEqual(receipt["outputExactAssetCount"], 1)
            self.assertEqual(
                set(receipt["inputs"]["placements"]),
                {"0/shared.placements.json", "1/shared.placements.json"},
            )
            self.assertEqual(
                receipt["inputs"]["placementDirectories"],
                [first_directory.as_posix(), second_directory.as_posix()],
            )

            arguments.include_level = []
            arguments.include_source_id = ["source_b"]
            arguments.catalog_output = root / "catalog-source.txt"
            arguments.placement_output = root / "placements-source.txt"
            arguments.receipt_output = root / "receipt-source.json"
            second_receipt = compile_scene(arguments)
            second_catalog = arguments.catalog_output.read_text(encoding="utf-8")
            self.assertEqual(second_receipt["outputScope"], "fixture")
            self.assertIn('"asset_b"', second_catalog)
            self.assertNotIn('"asset_a"', second_catalog)

            arguments.expect_output_assets = 2
            with self.assertRaisesRegex(ValueError, "output asset count mismatch: 1"):
                compile_scene(arguments)

    def test_single_directory_namespace_remains_backward_compatible(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            asset_manifest = root / "assets.json"
            runtime_manifest = root / "runtime.json"
            runtime_root = root / "runtime"
            placements_directory = root / "placements"
            runtime_root.mkdir()
            asset = self.asset("asset_a")
            self.write_json(asset_manifest, {"areaId": "TEST", "assets": [asset]})
            self.write_json(
                runtime_manifest,
                {
                    "areaId": "TEST",
                    "assets": [{"assetId": "asset_a", "model": "asset_a.wmodel"}],
                },
            )
            (runtime_root / "asset_a.wmodel").write_bytes(b"WMOD-a")
            self.write_json(
                placements_directory / "single.placements.json",
                {
                    "schemaVersion": 1,
                    "propertyErrors": [],
                    "placements": [self.placement("source_a", "asset_a", "LEVEL_A")],
                },
            )

            arguments = self.arguments(
                root,
                asset_manifest,
                runtime_manifest,
                runtime_root,
                placements_directory,
            )
            receipt = compile_scene(arguments)
            self.assertEqual(receipt["assetCount"], 1)
            self.assertEqual(
                set(receipt["inputs"]["placements"]),
                {"single.placements.json"},
            )

    def test_v2_joins_same_mesh_path_by_ordered_material_signature(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            runtime_root = root / "runtime"
            placements_directory = root / "placements"
            runtime_root.mkdir()
            slots = [
                {
                    "slot": 0,
                    "packageIndex": -1,
                    "class": "MaterialInstanceConstant",
                    "objectPath": "TEST.Material.Curtain_MI",
                },
                {
                    "slot": 1,
                    "packageIndex": 0,
                    "class": None,
                    "objectPath": None,
                },
            ]
            override_signature = material_signature_from_slots(slots)
            full_path = "test.mesh.shared_mesh"
            assets = [
                self.asset(
                    "asset_base",
                    full_path=full_path,
                    material_signature=EMPTY_MATERIAL_SIGNATURE,
                ),
                self.asset(
                    "asset_override",
                    full_path=full_path,
                    material_signature=override_signature,
                ),
            ]
            asset_manifest = root / "assets.json"
            runtime_manifest = root / "runtime.json"
            self.write_json(asset_manifest, {"areaId": "TEST", "assets": assets})
            self.write_json(
                runtime_manifest,
                {
                    "areaId": "TEST",
                    "assets": [
                        {"assetId": "asset_base", "model": "base.wmodel"},
                        {"assetId": "asset_override", "model": "override.wmodel"},
                    ],
                },
            )
            (runtime_root / "base.wmodel").write_bytes(b"WMOD-base")
            (runtime_root / "override.wmodel").write_bytes(b"WMOD-override")
            base_placement = self.placement("source_base", "ignored", "LEVEL_A")
            base_placement["asset"]["objectPath"] = full_path
            base_placement["materialOverrides"] = {
                "propertyPresent": False,
                "slots": [],
                "signatureSha256": EMPTY_MATERIAL_SIGNATURE,
            }
            override_placement = self.placement(
                "source_override", "ignored", "LEVEL_A"
            )
            override_placement["asset"]["objectPath"] = full_path
            override_placement["materialOverrides"] = {
                "propertyPresent": True,
                "slots": slots,
                "signatureSha256": override_signature,
            }
            self.write_json(
                placements_directory / "source.placements.json",
                {
                    "schemaVersion": 2,
                    "propertyErrors": [],
                    "unresolvedPlacements": [],
                    "placements": [base_placement, override_placement],
                },
            )

            arguments = self.arguments(
                root,
                asset_manifest,
                runtime_manifest,
                runtime_root,
                placements_directory,
            )
            receipt = compile_scene(arguments)
            rows = arguments.placement_output.read_text(encoding="utf-8").splitlines()
            self.assertIn(
                '"asset_base"', next(row for row in rows if '"source_base"' in row)
            )
            self.assertIn(
                '"asset_override"',
                next(row for row in rows if '"source_override"' in row),
            )
            self.assertEqual(2, receipt["exactAssetCount"])

    def test_v2_rejects_forged_material_signature(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            runtime_root = root / "runtime"
            placements_directory = root / "placements"
            runtime_root.mkdir()
            asset_manifest = root / "assets.json"
            runtime_manifest = root / "runtime.json"
            self.write_json(
                asset_manifest,
                {"areaId": "TEST", "assets": [self.asset("asset_a")]},
            )
            self.write_json(
                runtime_manifest,
                {
                    "areaId": "TEST",
                    "assets": [{"assetId": "asset_a", "model": "asset_a.wmodel"}],
                },
            )
            (runtime_root / "asset_a.wmodel").write_bytes(b"WMOD-a")
            placement = self.placement("source_a", "asset_a", "LEVEL_A")
            placement["materialOverrides"] = {
                "propertyPresent": False,
                "slots": [],
                "signatureSha256": "0" * 64,
            }
            self.write_json(
                placements_directory / "source.placements.json",
                {
                    "schemaVersion": 2,
                    "propertyErrors": [],
                    "unresolvedPlacements": [],
                    "placements": [placement],
                },
            )
            arguments = self.arguments(
                root,
                asset_manifest,
                runtime_manifest,
                runtime_root,
                placements_directory,
            )
            with self.assertRaisesRegex(ValueError, "signature mismatch"):
                compile_scene(arguments)

    def test_parser_accepts_repeated_placements_dir(self):
        argv = [
            "build_maptool_scene.py",
            "--area-id", "TEST",
            "--asset-manifest", "assets.json",
            "--runtime-manifest", "runtime.json",
            "--runtime-root", "runtime",
            "--placements-dir", "first",
            "--placements-dir", "second",
            "--catalog-output", "catalog.txt",
            "--placement-output", "placements.txt",
            "--receipt-output", "receipt.json",
            "--expect-output-assets", "7",
        ]
        with patch("sys.argv", argv):
            arguments = parse_args()
        self.assertEqual(arguments.placements_dir, [Path("first"), Path("second")])
        self.assertEqual(arguments.expect_output_assets, 7)

    def test_overlay_reuses_exact_asset_while_visibility_override_hides_source(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            asset_manifest = root / "assets.json"
            runtime_manifest = root / "runtime.json"
            overlay_manifest = root / "overlay.json"
            render_profile_manifest = root / "render-profile.json"
            runtime_root = root / "runtime"
            placements_directory = root / "placements"
            runtime_root.mkdir()

            asset = self.asset("asset_a")
            self.write_json(asset_manifest, {"areaId": "TEST", "assets": [asset]})
            self.write_json(
                runtime_manifest,
                {
                    "areaId": "TEST",
                    "assets": [{"assetId": "asset_a", "model": "asset_a.wmodel"}],
                },
            )
            (runtime_root / "asset_a.wmodel").write_bytes(b"WMOD-a")
            self.write_json(
                placements_directory / "source.placements.json",
                {
                    "schemaVersion": 1,
                    "propertyErrors": [],
                    "placements": [self.placement("source_a", "asset_a", "SOURCE")],
                },
            )
            self.write_json(
                overlay_manifest,
                {
                    "schemaVersion": 1,
                    "areaId": "TEST",
                    "assets": [],
                    "placements": [
                        {
                            "placementId": 7,
                            "sourcePlacementId": "registration:source_a",
                            "sourceLevel": "REGISTERED",
                            "transformSource": "overlay",
                            "assetId": "asset_a",
                            "position": [0.0, 10.0, 0.0],
                            "quaternion": [0.0, 0.0, 0.0, 1.0],
                            "scale": [1.0, 1.0, 1.0],
                            "visible": True,
                        }
                    ],
                },
            )
            self.write_json(
                render_profile_manifest,
                {
                    "schemaVersion": 1,
                    "areaId": "TEST",
                    "profiles": [],
                    "visibilityOverrides": [
                        {"sourcePlacementId": "source_a", "visible": False}
                    ],
                },
            )

            arguments = self.arguments(
                root,
                asset_manifest,
                runtime_manifest,
                runtime_root,
                placements_directory,
            )
            arguments.overlay_manifest = overlay_manifest
            arguments.render_profile_manifest = render_profile_manifest
            arguments.expect_assets = 1
            arguments.expect_source_placements = 1
            arguments.expect_overlay_assets = 0
            arguments.expect_overlay_placements = 1
            arguments.expect_output_placements = 2
            receipt = compile_scene(arguments)

            lines = arguments.placement_output.read_text(encoding="utf-8").splitlines()
            self.assertEqual(lines[0], 'LOSTARK_MAP_PLACEMENTS 2 "TEST" 2')
            self.assertTrue(next(line for line in lines if '"source_a"' in line).endswith(" 0"))
            self.assertTrue(
                next(line for line in lines if '"registration:source_a"' in line).endswith(" 1")
            )
            self.assertEqual(receipt["visibilityOverrideCount"], 1)
            self.assertEqual(receipt["overlayPlacementCount"], 1)


if __name__ == "__main__":
    unittest.main()
