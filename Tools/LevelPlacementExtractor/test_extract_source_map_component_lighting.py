import hashlib
import struct
import unittest

import extract_source_map_component_lighting as subject


def reference(index):
    names = {1: "normalizedaveragecolor0_12", 2: "directionalmaxcomponent0_12", 3: "third0_12"}
    return {"packageIndex": index, "objectPath": names.get(index),
            "sourceObject": f"MAP.{names[index]}" if index else None,
            "className": "lightmaptexture2d" if index else None}


def rnm_tail(color=b"", third=0, coordinates=(0.25, 0.5, 0.125, 0.25)):
    value = bytearray(struct.pack("<5i", 1, 0, 0, 2, 1))
    value.extend(bytes(range(16)))
    for index, scale in ((1, (1, 1, 1)), (2, (7, 5, 6)), (third, (2, 2, 2))):
        value.extend(struct.pack("<i3f", index, *scale))
    value.extend(struct.pack("<4f", *coordinates))
    value.append(1 if color else 0)
    if color:
        value.extend(struct.pack("<4I", 4, len(color) // 4, 4, len(color) // 4))
        value.extend(color)
    value.extend(bytes(4))
    return bytes(value)


class ComponentLightingTests(unittest.TestCase):
    def test_rnm_all_coefficients_guids_uv_and_native_color_are_exact(self):
        color = bytes([0, 1, 2, 3, 254, 253, 252, 251])
        result = subject.decode_native_lighting(rnm_tail(color), reference)
        self.assertEqual(result["averageTexture"], "MAP.normalizedaveragecolor0_12")
        self.assertEqual(result["directionalScale"], [7.0, 5.0, 6.0])
        self.assertEqual(result["coefficients"][2]["scale"], [2.0, 2.0, 2.0])
        self.assertEqual(result["bakedLightGuids"], [bytes(range(16)).hex()])
        self.assertEqual(result["coordinateScale"], [0.25, 0.5])
        self.assertEqual(result["coordinateBias"], [0.125, 0.25])
        self.assertEqual(result["colorVertexCount"], 2)
        self.assertEqual(result["colorSHA256"], hashlib.sha256(color).hexdigest())
        self.assertTrue(result["nativeTailCompletelyConsumed"])

    def test_no_lod_and_exact_null_lightmap_preserve_distinct_source_absence(self):
        self.assertEqual(subject.decode_native_lighting(bytes(4), reference)["status"], "NO_LOD_LIGHTING_DATA")
        no_lightmap = struct.pack("<4i", 1, 0, 0, 0) + bytes(5)
        null = subject.decode_native_lighting(no_lightmap, reference)
        self.assertEqual(null["status"], "NULL_LIGHTMAP")
        self.assertEqual(null["lodCount"], 1)
        self.assertEqual(null["lightMapKind"], 0)
        self.assertTrue(null["nativeTailCompletelyConsumed"])
        for unknown in (no_lightmap+b"\0", no_lightmap[:-1]+b"\1", no_lightmap[:16]+b"\1"+bytes(4)):
            with self.assertRaises(subject.UnsupportedNative):
                subject.decode_native_lighting(unknown, reference)
        with self.assertRaises(ValueError):
            subject.decode_native_lighting(bytes(8), reference)

    def test_vertex_shadow_third_channel_and_unknown_terminal_are_not_silent_absence(self):
        vertex = struct.pack("<4i", 1, 0, 0, 1) + b"unknown vertex stream"
        shadow = struct.pack("<4i", 1, 1, 1, 0) + b"unknown shadow stream"
        for tail in (vertex, shadow, rnm_tail(third=3), rnm_tail() + b"new ABI tail"):
            with self.subTest(tail=tail[:20]):
                with self.assertRaises(subject.UnsupportedNative) as context:
                    subject.decode_native_lighting(tail, reference)
                self.assertGreater(context.exception.offset, 0)
                self.assertTrue(context.exception.decoded)

    def test_invalid_counts_uv_and_truncation_are_failures(self):
        for tail in (struct.pack("<i", -1), struct.pack("<2i", 1, 5000),
                     rnm_tail(coordinates=(float("nan"), 0.5, 0, 0)),
                     rnm_tail(coordinates=(1.1, 0.5, 0, 0)), rnm_tail()[:30]):
            with self.subTest(tail=tail[:20]):
                with self.assertRaises((ValueError, subject.ue.ExtractionError)):
                    subject.decode_native_lighting(tail, reference)

    def test_environment_zero_and_empty_array_are_serialized_values(self):
        extractor = subject.ComponentLightingExtractor.__new__(subject.ComponentLightingExtractor)
        props = {"pbrenvironmentmapoverrides": {"value": []},
                 "pbrenvironmentcoloroverride": {"structType": "linearcolor", "value": {"hex": bytes(16).hex()}},
                 "pbrenvironmentcubemapangleoverride": {"value": 0.0}}
        result = extractor.environment(props)
        self.assertTrue(result["maps"]["propertyPresent"])
        self.assertEqual(result["maps"]["references"], [])
        self.assertEqual(result["color"]["value"], [0.0] * 4)
        self.assertEqual(result["angleDegrees"]["value"], 0.0)
        self.assertFalse(result["componentMinRoughnessTag"]["propertyPresent"])
        self.assertIsNone(result["componentMinRoughnessTag"]["value"])

    def test_missing_component_tag_differs_from_unread_external_cube_property(self):
        extractor = subject.ComponentLightingExtractor.__new__(subject.ComponentLightingExtractor)
        extractor.reference = lambda index: {"packageIndex": index, "sourceObject": "source.tex.cube", "className": "texturecube"}
        extractor.texture = lambda _: None
        result = extractor.environment({"pbrenvironmentmapoverrides": {"value": [-1]}})
        self.assertEqual(result["angleDegrees"]["status"], "INSTANCE_PROPERTY_ABSENT")
        self.assertEqual(result["cubeInputs"][0]["minimumRoughness"]["status"], "EXTERNAL_TEXTURE_PROPERTY_NOT_READ")
        self.assertIsNone(result["cubeInputs"][0]["minimumRoughness"]["propertyPresent"])


if __name__ == "__main__":
    unittest.main()
