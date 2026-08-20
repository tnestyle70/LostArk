import argparse
import json
import math
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from build_maptool_scene import (
    IMPORTED_ID_BIT,
    classify_non_visual_asset,
    compile_scene,
    convert_position,
    convert_rotation,
    convert_scale,
    directx_row_matrix_from_quaternion,
    imported_id,
    is_non_visual_helper_asset,
    parse_args,
    scale_flags,
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

    def test_lv_navimesh_assets_are_non_visual_helpers(self):
        asset = {
            "rootImport": "lv_navimesh",
            "logicalPackage": "LV_NAVIMESH",
            "fullPath": "lv_navimesh.mesh.lv_common_mesh_cul_box_8",
        }
        self.assertTrue(is_non_visual_helper_asset(asset))
        self.assertEqual(classify_non_visual_asset(asset), "nav-helper")

    def test_lv_module_proxy_is_hidden_until_special_surface_support(self):
        asset = {
            "rootImport": "lv_module",
            "logicalPackage": "LV_MODULE",
            "objectName": "lv_module_water02_512",
            "fullPath": "lv_module.mesh.lv_module_water02_512",
        }
        self.assertEqual(classify_non_visual_asset(asset), "module-proxy")

    def test_heartrb_water_is_deferred_instead_of_rendered_gray(self):
        asset = {
            "rootImport": "lv_lut_heartrb",
            "logicalPackage": "LV_LUT_HEARTRB",
            "objectName": "lv_lut_heartrb_water01_sm",
            "fullPath": "lv_lut_heartrb.mesh.lv_lut_heartrb_water01_sm",
        }
        self.assertEqual(classify_non_visual_asset(asset), "deferred-water")

    def test_bfx_mesh_is_deferred_instead_of_rendered_opaque(self):
        asset = {
            "rootImport": "bfx_sm_00",
            "logicalPackage": "BFX_SM_00",
            "objectName": "bfm_mossfog_001",
            "fullPath": "bfx_sm_00.bfm_mossfog_001",
        }
        self.assertEqual(classify_non_visual_asset(asset), "deferred-fx")

    def test_regular_level_asset_remains_visible(self):
        asset = {
            "rootImport": "pvp_retown_a",
            "logicalPackage": "PVP_RETOWN_A",
            "fullPath": "pvp_retown_a.mesh.bg_pvp_retown_floor01_sm",
        }
        self.assertFalse(is_non_visual_helper_asset(asset))


class MapToolSceneCompileTests(unittest.TestCase):
    @staticmethod
    def write_json(path: Path, value) -> None:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(value), encoding="utf-8")

    @staticmethod
    def asset(asset_id: str) -> dict:
        return {
            "assetId": asset_id,
            "objectName": asset_id,
            "fullPath": f"test.mesh.{asset_id}",
            "sourceCategory": "StaticMesh",
            "logicalPackage": "TEST",
            "rootImport": "TEST",
        }

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
