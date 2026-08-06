import csv
import hashlib
import struct
import tempfile
import unittest
from pathlib import Path

from extract_ue3_particle_graph import (
    is_particle_graph_class,
    iter_property_reference_values,
    parse_physical_package_overrides,
)
from extract_ue3_placements import decode_property_value
from extract_ue3_effect_semantics import encode_vector_field
from stage_ue3_source_pack import load_inventory, resolve_package, sha256_file


def encode_fname(index: int) -> bytes:
    return struct.pack("<2i", index, 0)


def encode_property(
    name_index: int, type_index: int, payload: bytes
) -> bytes:
    return (
        encode_fname(name_index)
        + encode_fname(type_index)
        + struct.pack("<2i", len(payload), 0)
        + payload
    )


class EffectExtractionToolTests(unittest.TestCase):
    def test_particle_graph_class_filter_keeps_supported_ue3_graph_nodes(self):
        self.assertTrue(is_particle_graph_class("ParticleSystem"))
        self.assertTrue(is_particle_graph_class("ParticleSpriteEmitter"))
        self.assertTrue(is_particle_graph_class("ParticleModuleTypeDataMesh"))
        self.assertTrue(is_particle_graph_class("EFParticleModuleTypeDataDecal"))
        self.assertTrue(is_particle_graph_class("DistributionVectorConstantCurve"))
        self.assertFalse(is_particle_graph_class("CameraActor"))

    def test_inventory_resolution_preserves_logical_physical_and_size(self):
        with tempfile.TemporaryDirectory() as temporary:
            inventory_path = Path(temporary) / "packages.csv"
            with inventory_path.open("w", encoding="utf-8", newline="") as output:
                writer = csv.DictWriter(
                    output,
                    fieldnames=("logical_name", "physical_file", "byte_size"),
                )
                writer.writeheader()
                writer.writerow(
                    {
                        "logical_name": "FX_PC_SWP_00",
                        "physical_file": "opaque.upk",
                        "byte_size": "123",
                    }
                )

            inventory = load_inventory(inventory_path)
            resolved = resolve_package(inventory, "fx_pc_swp_00", "core")
            unresolved = resolve_package(inventory, "missing", "dependency")

            self.assertEqual("FX_PC_SWP_00", resolved["logicalPackage"])
            self.assertEqual("opaque.upk", resolved["physicalPackage"])
            self.assertEqual(123, resolved["inventoryByteSize"])
            self.assertTrue(resolved["resolved"])
            self.assertFalse(unresolved["resolved"])

    def test_sha256_receipt_hashes_full_payload(self):
        with tempfile.TemporaryDirectory() as temporary:
            payload = Path(temporary) / "payload.upk"
            payload.write_bytes(b"lostark-effect-source\0payload")
            expected = hashlib.sha256(payload.read_bytes()).hexdigest()
            self.assertEqual(expected, sha256_file(payload))

    def test_physical_package_override_preserves_exact_identified_file(self):
        with tempfile.TemporaryDirectory() as temporary:
            package = Path(temporary) / "opaque.upk"
            package.write_bytes(b"package")
            overrides = parse_physical_package_overrides(
                [f"FX_BS_03={package}"]
            )
            self.assertEqual(package, overrides["fx_bs_03"])

    def test_raw_distribution_recovers_nested_distribution_reference(self):
        names = ["None", "Distribution", "ObjectProperty"]
        payload = (
            encode_property(1, 2, struct.pack("<i", 7))
            + encode_fname(0)
        )
        decoded = decode_property_value(
            "StructProperty",
            "RawDistributionFloat",
            payload,
            names,
            None,
        )
        self.assertEqual(
            7,
            decoded["properties"]["Distribution"]["value"],
        )
        properties = {
            "Lifetime": {
                "type": "StructProperty",
                "structType": "RawDistributionFloat",
                "value": decoded,
            }
        }
        self.assertEqual(
            [("Lifetime.Distribution", 7)],
            list(iter_property_reference_values(properties)),
        )

    def test_raw_distribution_lookup_table_is_decoded_as_floats(self):
        payload = struct.pack("<i3f", 3, -1.0, 0.25, 2.0)
        decoded = decode_property_value(
            "ArrayProperty",
            None,
            payload,
            [],
            None,
            "LookupTable",
            "RawDistributionFloat",
        )
        self.assertEqual([-1.0, 0.25, 2.0], decoded)

    def test_tagged_struct_array_decodes_particle_burst(self):
        names = [
            "None",
            "Count",
            "CountLow",
            "Time",
            "IntProperty",
            "FloatProperty",
        ]

        def encode_int(name_index: int, value: int) -> bytes:
            return (
                encode_fname(name_index)
                + encode_fname(4)
                + struct.pack("<2i", 4, 0)
                + b"\0" * 8
                + struct.pack("<i", value)
            )

        burst = b"".join(
            (
                encode_int(1, 9),
                encode_int(2, -1),
                encode_property(3, 5, struct.pack("<f", 0.2)),
                encode_fname(0),
            )
        )
        decoded = decode_property_value(
            "ArrayProperty",
            None,
            struct.pack("<i", 1) + burst,
            names,
            None,
            "BurstList",
        )
        self.assertEqual(1, len(decoded))
        self.assertEqual(9, decoded[0]["Count"]["value"])
        self.assertEqual(-1, decoded[0]["CountLow"]["value"])
        self.assertAlmostEqual(0.2, decoded[0]["Time"]["value"], places=6)

    def test_only_known_object_arrays_are_treated_as_references(self):
        properties = {
            "Emitters": {
                "type": "ArrayProperty",
                "structType": None,
                "value": [3],
            },
            "LODDistances": {
                "type": "ArrayProperty",
                "structType": None,
                "value": [1176256512],
            },
        }
        self.assertEqual(
            [("Emitters", 3)], list(iter_property_reference_values(properties))
        )

    def test_interp_curve_float_recovers_tagged_keyframes(self):
        names = [
            "None",
            "Points",
            "ArrayProperty",
            "InVal",
            "FloatProperty",
            "OutVal",
        ]
        point = (
            encode_property(3, 4, struct.pack("<f", 0.5))
            + encode_property(5, 4, struct.pack("<f", 2.0))
            + encode_fname(0)
        )
        points = struct.pack("<i", 1) + point
        payload = encode_property(1, 2, points) + encode_fname(0)
        decoded = decode_property_value(
            "StructProperty",
            "InterpCurveFloat",
            payload,
            names,
            None,
        )
        keys = decoded["properties"]["Points"]["value"]
        self.assertEqual(1, len(keys))
        self.assertAlmostEqual(0.5, keys[0]["InVal"]["value"])
        self.assertAlmostEqual(2.0, keys[0]["OutVal"]["value"])

    def test_particle_random_seed_info_recovers_seed_array(self):
        names = ["None", "RandomSeeds", "ArrayProperty"]
        seeds = struct.pack("<i3i", 3, 17, -23, 991)
        payload = encode_property(1, 2, seeds) + encode_fname(0)
        decoded = decode_property_value(
            "StructProperty",
            "ParticleRandomSeedInfo",
            payload,
            names,
            None,
        )
        self.assertEqual(
            [17, -23, 991],
            decoded["properties"]["RandomSeeds"]["value"],
        )

    def test_material_input_recovers_expression_reference(self):
        names = ["None", "Expression", "ObjectProperty"]
        payload = (
            encode_property(1, 2, struct.pack("<i", -17))
            + encode_fname(0)
        )
        decoded = decode_property_value(
            "StructProperty",
            "ColorMaterialInput",
            payload,
            names,
            None,
        )
        self.assertEqual(
            -17,
            decoded["properties"]["Expression"]["value"],
        )
        properties = {
            "EmissiveColor": {
                "type": "StructProperty",
                "structType": "ColorMaterialInput",
                "value": decoded,
            }
        }
        self.assertEqual(
            [("EmissiveColor.Expression", -17)],
            list(iter_property_reference_values(properties)),
        )

    def test_vector_field_binary_preserves_dimensions_and_float_bits(self):
        words = [
            struct.unpack("<i", struct.pack("<f", value))[0]
            for value in (1.0, -2.0, 3.5)
        ]
        payload = encode_vector_field(1, 1, 1, words + [0])
        magic, version, size_x, size_y, size_z, sample_count = struct.unpack(
            "<4sIIIII", payload[:24]
        )
        self.assertEqual(b"WVF1", magic)
        self.assertEqual((1, 1, 1, 1, 1), (
            version, size_x, size_y, size_z, sample_count
        ))
        self.assertEqual((1.0, -2.0, 3.5), struct.unpack("<3f", payload[24:]))


if __name__ == "__main__":
    unittest.main()
