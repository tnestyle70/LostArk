import importlib.util
import math
import struct
import sys
import tempfile
import unittest
from pathlib import Path

SPEC = importlib.util.spec_from_file_location(
    "extract_ue3_landscape",
    Path(__file__).resolve().parent / "extract_ue3_landscape.py",
)
extract_ue3_landscape = importlib.util.module_from_spec(SPEC)
sys.modules["extract_ue3_landscape"] = extract_ue3_landscape
SPEC.loader.exec_module(extract_ue3_landscape)

block_compression_channel = extract_ue3_landscape.block_compression_channel
block_compression_colors = extract_ue3_landscape.block_compression_colors
decode_dds = extract_ue3_landscape.decode_dds
decode_inline_bulk_array = extract_ue3_landscape.decode_inline_bulk_array
cliff_blend_weight = extract_ue3_landscape.cliff_blend_weight
cliff_face_uv = extract_ue3_landscape.cliff_face_uv
is_ue3_landscape_hole = extract_ue3_landscape.is_ue3_landscape_hole
material_texture_file = extract_ue3_landscape.material_texture_file
quad_is_hole = extract_ue3_landscape.quad_is_hole
rgb565_triplet = extract_ue3_landscape.rgb565_triplet
triangle_is_cliff = extract_ue3_landscape.triangle_is_cliff


def channel_block(first: int, second: int, indices: int) -> bytes:
    return bytes([first, second]) + indices.to_bytes(6, "little")


def color_block(endpoint0: int, endpoint1: int, indices: int) -> bytes:
    return struct.pack("<HHI", endpoint0, endpoint1, indices)


def dds_file(four_cc: bytes, width: int, height: int, payload: bytes) -> bytes:
    header = bytearray(124)
    struct.pack_into("<I", header, 0, 124)
    struct.pack_into("<II", header, 8, height, width)
    struct.pack_into("<I", header, 72, 32)
    header[80:84] = four_cc
    return b"DDS " + bytes(header) + payload


def landscape_component_with_hole_weight(weight: int):
    mip = extract_ue3_landscape.TextureMip(
        level=0,
        width=3,
        height=3,
        bulk_flags=0,
        element_count=36,
        size_on_disk=36,
        logical_offset=0,
        bgra=bytes([0, 0, weight, 0]) * 9,
    )
    texture = extract_ue3_landscape.DecodedTexture(
        export_index=0,
        object_name="hole",
        pixel_format="PF_A8R8G8B8",
        source_art_flags=0,
        source_art_element_count=0,
        source_art_size_on_disk=0,
        source_art_offset=0,
        mips=(mip,),
    )
    return extract_ue3_landscape.LandscapeComponent(
        logical_package="TEST",
        export_index=0,
        object_name="component",
        section_base_x=0,
        section_base_y=0,
        component_size_quads=2,
        subsection_size_quads=2,
        num_subsections=1,
        heightmap_ref=0,
        heightmap_name="height",
        heightmap_path="height",
        heightmap_scale_bias=(1.0 / 3.0, 1.0 / 3.0, 0.0, 0.0),
        weightmap_refs=[0],
        weightmap_names=["hole"],
        weightmap_paths=["hole"],
        weightmap_scale_bias=(1.0 / 3.0, 1.0 / 3.0, 1.0 / 6.0, 1.0 / 6.0),
        weightmap_subsection_offset=0.5,
        allocations=[extract_ue3_landscape.LayerAllocation("__DataLayer__", 0, 0)],
        material_instance_ref=0,
        material_instance_name="material",
        cached_local_box={"min": [0.0, 0.0, 0.0], "max": [0.0, 0.0, 0.0]},
        weight_textures=[texture],
    )


class LandscapeHoleContractTests(unittest.TestCase):
    def test_ue3_hole_threshold_is_strictly_greater_than_170(self):
        self.assertFalse(is_ue3_landscape_hole(0))
        self.assertFalse(is_ue3_landscape_hole(170))
        self.assertTrue(is_ue3_landscape_hole(171))
        self.assertTrue(is_ue3_landscape_hole(255))

    def test_top_left_sample_owns_the_render_quad(self):
        component = landscape_component_with_hole_weight(171)
        self.assertTrue(quad_is_hole(component, 0, 0))
        component = landscape_component_with_hole_weight(170)
        self.assertFalse(quad_is_hole(component, 0, 0))


class CollisionHeightContractTests(unittest.TestCase):
    def test_inline_uint16_bulk_preserves_all_values(self):
        values = (30055, 32768, 36361)
        payload = struct.pack("<3H", *values)
        serial_offset = 100
        serial = struct.pack("<4i", 0, len(values), len(payload), 116) + payload
        actual, offset = decode_inline_bulk_array(
            serial, 0, serial_offset, "H", "test heights"
        )
        self.assertEqual(actual, values)
        self.assertEqual(offset, len(serial))

    def test_wrong_logical_offset_is_rejected(self):
        serial = struct.pack("<4iH", 0, 1, 2, 0, 32768)
        with self.assertRaises(extract_ue3_landscape.LandscapeError):
            decode_inline_bulk_array(serial, 0, 100, "H", "test heights")


class CliffBlendContractTests(unittest.TestCase):
    def test_flat_and_steep_endpoints_are_exact(self):
        self.assertEqual(cliff_blend_weight(1.0), 0.0)
        self.assertEqual(cliff_blend_weight(0.75), 0.0)
        self.assertEqual(cliff_blend_weight(0.35), 1.0)
        self.assertEqual(cliff_blend_weight(0.0), 1.0)

    def test_transition_is_smooth_and_bounded(self):
        self.assertAlmostEqual(cliff_blend_weight(0.55), 0.5)
        for value in (0.35, 0.4, 0.5, 0.6, 0.7, 0.75):
            self.assertGreaterEqual(cliff_blend_weight(value), 0.0)
            self.assertLessEqual(cliff_blend_weight(value), 1.0)

    def test_geometry_routes_only_faces_below_flat_threshold_to_cliff(self):
        self.assertFalse(triangle_is_cliff((0.0, 0.75, 0.25)))
        self.assertFalse(triangle_is_cliff((0.0, 1.0, 0.0)))
        self.assertTrue(triangle_is_cliff((0.0, 0.74, 0.25)))
        self.assertTrue(triangle_is_cliff((1.0, 0.0, 0.0)))

    def test_cliff_uv_uses_world_height(self):
        layer = {"tiling": 3.0, "rotation": 0.0}
        lower = cliff_face_uv(
            (10.0, 2.0, 5.0), (1.0, 0.0, 0.0), (40.0, 40.0), layer
        )
        upper = cliff_face_uv(
            (10.0, 12.0, 5.0), (1.0, 0.0, 0.0), (40.0, 40.0), layer
        )
        self.assertEqual(lower[0], upper[0])
        self.assertNotEqual(lower[1], upper[1])


class DeterministicLayerSourceTests(unittest.TestCase):
    def test_material_texture_file_selects_only_explicit_dds_export(self):
        with tempfile.TemporaryDirectory() as directory:
            package_root = Path(directory) / "BG_LANDSCAPE_B" / "Texture2D"
            package_root.mkdir(parents=True)
            dds_path = package_root / "layer_normal.dds"
            dds_path.write_bytes(b"dds")
            (package_root / "layer_normal.tga").write_bytes(b"tga")

            selected = material_texture_file(
                Path(directory),
                "IGNORED_PACKAGE",
                "BG_LANDSCAPE_B.Texture2D.layer_normal",
            )
            self.assertEqual(selected, dds_path)


class BlockCompressionChannelTests(unittest.TestCase):
    def test_eight_value_mode_weights_total_seven(self):
        # a0 > a1 selects the eight value palette from the BC4 specification.
        actual = [
            block_compression_channel(channel_block(255, 0, index))[0]
            for index in range(8)
        ]
        self.assertEqual(actual, [255, 0, 218, 182, 145, 109, 72, 36])

    def test_six_value_mode_uses_explicit_zero_and_full(self):
        # a0 <= a1 selects the six value palette plus explicit 0 and 255.
        actual = [
            block_compression_channel(channel_block(0, 255, index))[0]
            for index in range(8)
        ]
        self.assertEqual(actual, [0, 255, 51, 102, 153, 204, 0, 255])

    def test_endpoints_are_preserved_exactly(self):
        for first, second in ((255, 0), (0, 255), (200, 40), (40, 200)):
            table = [
                block_compression_channel(channel_block(first, second, index))[0]
                for index in range(2)
            ]
            self.assertEqual(table, [first, second])

    def test_interpolated_values_stay_inside_the_endpoint_range(self):
        values = [
            block_compression_channel(channel_block(200, 40, index))[0]
            for index in range(8)
        ]
        for value in values:
            self.assertGreaterEqual(value, 40)
            self.assertLessEqual(value, 200)

    def test_indices_select_per_texel(self):
        # Texel 0 uses index 0 and texel 1 uses index 1.
        decoded = block_compression_channel(channel_block(255, 0, 0b001_000))
        self.assertEqual(decoded[0], 255)
        self.assertEqual(decoded[1], 0)


class BlockCompressionColorTests(unittest.TestCase):
    def test_rgb565_expands_to_full_range(self):
        self.assertEqual(rgb565_triplet(0xFFFF), (255, 255, 255))
        self.assertEqual(rgb565_triplet(0x0000), (0, 0, 0))
        self.assertEqual(rgb565_triplet(0xF800), (255, 0, 0))
        self.assertEqual(rgb565_triplet(0x001F), (0, 0, 255))

    def test_four_colour_mode_weights_total_three(self):
        texels = block_compression_colors(
            color_block(0xF800, 0x001F, 0b11_10_01_00), False
        )
        self.assertEqual(texels[0], (255, 0, 0, 255))
        self.assertEqual(texels[1], (0, 0, 255, 255))
        self.assertEqual(texels[2], (170, 0, 85, 255))
        self.assertEqual(texels[3], (85, 0, 170, 255))

    def test_three_colour_mode_has_transparent_fourth_entry(self):
        texels = block_compression_colors(
            color_block(0x001F, 0xF800, 0b11_10_01_00), False
        )
        self.assertEqual(texels[2], (127, 0, 127, 255))
        self.assertEqual(texels[3], (0, 0, 0, 0))

    def test_forced_four_colour_mode_ignores_endpoint_order(self):
        # DXT5 always uses the four colour palette regardless of endpoint order.
        texels = block_compression_colors(
            color_block(0x001F, 0xF800, 0b11_10_01_00), True
        )
        self.assertEqual(texels[3], (170, 0, 85, 255))


class DecodeDdsTests(unittest.TestCase):
    def test_dxt1_block_decodes_to_expected_colours(self):
        payload = color_block(0xF800, 0x001F, 0)
        image = decode_dds_bytes(dds_file(b"DXT1", 4, 4, payload))
        self.assertEqual(image.width, 4)
        self.assertEqual(image.height, 4)
        self.assertEqual(set(image.pixels), {(255, 0, 0, 255)})

    def test_dxt5_alpha_is_taken_from_the_channel_block(self):
        payload = channel_block(255, 0, 0) + color_block(0xF800, 0x001F, 0)
        image = decode_dds_bytes(dds_file(b"DXT5", 4, 4, payload))
        self.assertEqual(set(image.pixels), {(255, 0, 0, 255)})

        payload = channel_block(0, 255, 0) + color_block(0xF800, 0x001F, 0)
        image = decode_dds_bytes(dds_file(b"DXT5", 4, 4, payload))
        self.assertEqual(set(image.pixels), {(255, 0, 0, 0)})

    def test_ati2_reconstructs_a_unit_normal(self):
        # Both channels at the midpoint mean X and Y are ~0 so Z must be ~1.
        payload = channel_block(128, 128, 0) + channel_block(128, 128, 0)
        image = decode_dds_bytes(dds_file(b"ATI2", 4, 4, payload))
        for red, green, blue, alpha in image.pixels:
            normal_x = red / 127.5 - 1.0
            normal_y = green / 127.5 - 1.0
            normal_z = blue / 127.5 - 1.0
            length = math.sqrt(
                normal_x * normal_x + normal_y * normal_y + normal_z * normal_z
            )
            self.assertAlmostEqual(length, 1.0, places=2)
            self.assertEqual(alpha, 255)

    def test_unsupported_fourcc_is_rejected(self):
        with self.assertRaises(extract_ue3_landscape.LandscapeError):
            decode_dds_bytes(dds_file(b"DXT2", 4, 4, bytes(16)))

    def test_truncated_payload_is_rejected(self):
        with self.assertRaises(extract_ue3_landscape.LandscapeError):
            decode_dds_bytes(dds_file(b"DXT1", 8, 8, bytes(8)))


def decode_dds_bytes(data: bytes):
    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory) / "sample.dds"
        path.write_bytes(data)
        return decode_dds(path)


if __name__ == "__main__":
    unittest.main()
