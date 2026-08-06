from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path

import build_bern_castle_nonstatic_manifest as tool


def minimal_manifest() -> dict:
    item = {
        "id": tool.stable_id("water", "LV_BER_BERNCASTLE_T_SL00", "placement:1"),
        "type": "water",
        "source": {
            "level": "LV_BER_BERNCASTLE_T_SL00",
            "placementId": "placement:1",
        },
        "missingReferences": [],
        "payload": {"schema": "test"},
    }
    return {
        "schemaVersion": 1,
        "areaId": tool.AREA_ID,
        "summary": {"sourcePackages": 1, "items": 1, "water": 1},
        "items": [item],
    }


class NonStaticManifestTests(unittest.TestCase):
    def test_stable_id_depends_on_full_source_identity(self) -> None:
        first = tool.stable_id("decal", "LV_BER_BERNCASTLE_T_SL00", "world.a")
        self.assertEqual(
            first,
            tool.stable_id("DECAL", "lv_ber_berncastle_t_sl00", "WORLD.A"),
        )
        self.assertNotEqual(
            first,
            tool.stable_id("decal", "LV_BER_BERNCASTLE_T_SL01", "world.a"),
        )

    def test_duplicate_source_locator_is_rejected(self) -> None:
        manifest = minimal_manifest()
        duplicate = dict(manifest["items"][0])
        duplicate["id"] = "BERN_NS_WATER_DIFFERENT"
        manifest["items"].append(duplicate)
        manifest["summary"]["items"] = 2
        with self.assertRaisesRegex(tool.ManifestError, "duplicate source locator"):
            tool.validate_manifest(
                manifest, {"sourcePackages": 1, "items": 2, "water": 1}
            )

    def test_expected_count_gate_is_strict(self) -> None:
        manifest = minimal_manifest()
        with self.assertRaisesRegex(tool.ManifestError, "expected count gate failed"):
            tool.validate_manifest(manifest, {"items": 2})

    def test_atomic_commit_round_trips_validated_document(self) -> None:
        manifest = minimal_manifest()
        expected = {"sourcePackages": 1, "items": 1, "water": 1}
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "manifest.json"
            tool.atomic_commit_json(output, manifest, expected)
            self.assertEqual(json.loads(output.read_text(encoding="utf-8")), manifest)
            self.assertEqual(list(output.parent.glob("*.staging")), [])

    def test_matrix_payload_keeps_transform_and_full_hex(self) -> None:
        values = [float(index) for index in range(16)]
        import struct

        payload = struct.pack("<16f", *values)
        decoded = tool._decode_property(
            "StructProperty", "Matrix", payload, [], None
        )
        self.assertEqual(decoded["translation"], {"x": 13.0, "y": 14.0, "z": 15.0})


if __name__ == "__main__":
    unittest.main()
