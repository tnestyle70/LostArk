import csv
import hashlib
import tempfile
import unittest
from pathlib import Path

from extract_ue3_particle_graph import is_particle_graph_class
from stage_ue3_source_pack import load_inventory, resolve_package, sha256_file


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


if __name__ == "__main__":
    unittest.main()
