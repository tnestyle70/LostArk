import math
import unittest

from build_maptool_scene import (
    IMPORTED_ID_BIT,
    classify_non_visual_asset,
    convert_position,
    convert_rotation,
    convert_scale,
    directx_row_matrix_from_quaternion,
    imported_id,
    is_non_visual_helper_asset,
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


if __name__ == "__main__":
    unittest.main()
