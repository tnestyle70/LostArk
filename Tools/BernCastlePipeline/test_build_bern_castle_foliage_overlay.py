from __future__ import annotations

import json
import struct
import tempfile
import unittest
from pathlib import Path
from unittest import mock

import build_bern_castle_foliage_overlay as tool


def matrix_values(
    sx: float,
    sy: float,
    sz: float,
    x: float,
    y: float,
    z: float,
) -> tuple[float, ...]:
    return (
        sx, 0.0, 0.0, 0.0,
        0.0, sy, 0.0, 0.0,
        0.0, 0.0, sz, 0.0,
        x, y, z, 1.0,
        0.125, 0.25, 0.375, 0.5,
    )


def native_suffix(
    instances: list[tuple[float, ...]],
    references: list[int],
    guid_count: int = 1,
) -> bytes:
    assert len(instances) == len(references)
    result = bytearray(struct.pack("<II", 1, len(instances)))
    result.extend(struct.pack("<" + "i" * len(references), *references))
    result.extend(struct.pack("<iiI", 0, 2, guid_count))
    result.extend(bytes(range(16)) * guid_count)
    result.extend(b"\0" * tool.NATIVE_FIXED_TAIL_SIZE)
    result.extend(struct.pack("<II", tool.INSTANCE_ELEMENT_SIZE, len(instances)))
    for values in instances:
        result.extend(struct.pack("<20f", *values))
    return bytes(result)


def foliage_item(
    item_id: str,
    export_index: int,
    serial_offset: int,
    serial: bytes,
    suffix: bytes,
    mesh_path: str,
    package_sha256: str,
    level: str,
) -> dict:
    property_end = len(serial) - len(suffix)
    return {
        "id": item_id,
        "type": "foliage",
        "source": {
            "level": level,
            "packageSha256": package_sha256,
            "component": {
                "exportIndex": export_index,
                "class": "instancedstaticmeshcomponent",
                "objectName": f"component_{export_index}",
                "objectPath": f"theworld.persistentlevel.actor.component_{export_index}",
                "serialOffset": serial_offset,
                "serialSize": len(serial),
            },
        },
        "references": [
            {
                "role": "staticMesh",
                "class": "StaticMesh",
                "objectPath": mesh_path,
            }
        ],
        "payload": {
            "parseStatus": "parsed",
            "serial": {
                "offset": serial_offset,
                "size": len(serial),
                "sha256": tool.sha256_bytes(serial),
            },
            "propertyStream": {"endWithinSerial": property_end},
            "nativeSuffix": {
                "offsetWithinSerial": property_end,
                "size": len(suffix),
                "sha256": tool.sha256_bytes(suffix),
                "prefixHex": suffix[:64].hex(),
            },
        },
    }


def fixture_documents() -> tuple[dict, dict, dict, dict, tool.PackageView, tool.Expectations]:
    level = "LV_BER_BERNCASTLE_T_TEST"
    package_sha256 = "A" * 64
    base_path = "pkg.mesh.base_tree"
    supplement_path = "pkg.mesh.supplement_tree"
    suffix_a = native_suffix([matrix_values(1, 1, 1, 100, 200, 300)], [3])
    suffix_b = native_suffix([matrix_values(-2, 3, 4, 400, 500, 600)], [4], 2)
    serial_a = b"PROPERTY" + suffix_a
    serial_b = b"PROPERTY" + suffix_b
    logical = serial_a + serial_b
    exports = (
        tool.ExportView(
            0,
            "instancedstaticmeshcomponent",
            "component_0",
            "theworld.persistentlevel.actor.component_0",
            0,
            len(serial_a),
        ),
        tool.ExportView(
            1,
            "instancedstaticmeshcomponent",
            "component_1",
            "theworld.persistentlevel.actor.component_1",
            len(serial_a),
            len(serial_b),
        ),
        tool.ExportView(2, "shadowmap2d", "shadow_0", "world.shadow_0", 0, 0),
        tool.ExportView(3, "shadowmap2d", "shadow_1", "world.shadow_1", 0, 0),
    )
    package = tool.PackageView(
        logical_name=level,
        physical_path="fixture.upk",
        physical_sha256=package_sha256,
        package_version=868,
        export_count=len(exports),
        logical=logical,
        exports=exports,
    )
    nonstatic = {
        "schemaVersion": 1,
        "areaId": tool.AREA_ID,
        "summary": {"foliage": 2},
        "sources": [{"logicalName": level}],
        "items": [
            foliage_item(
                "foliage-a", 0, 0, serial_a, suffix_a, base_path, package_sha256, level
            ),
            foliage_item(
                "foliage-b",
                1,
                len(serial_a),
                serial_b,
                suffix_b,
                supplement_path,
                package_sha256,
                level,
            ),
        ],
    }
    base_static = {
        "schemaVersion": 1,
        "areaId": tool.AREA_ID,
        "assetCount": 1,
        "assets": [{"assetId": "BASE_TREE", "fullPath": base_path}],
    }
    base_runtime = {
        "schemaVersion": 1,
        "areaId": tool.AREA_ID,
        "assetCount": 1,
        "assets": [
            {
                "assetId": "BASE_TREE",
                "fullPath": base_path,
                "model": "BASE_TREE/BASE_TREE.wmodel",
            }
        ],
    }
    supplement_runtime = {
        "schemaVersion": 1,
        "areaId": tool.AREA_ID,
        "kind": "foliage-staticmesh-supplement-runtime",
        "assetCount": 1,
        "usageCount": 1,
        "assets": [
            {
                "assetId": "SUPPLEMENT_TREE",
                "fullPath": supplement_path,
                "model": "SUPPLEMENT_TREE/SUPPLEMENT_TREE.wmodel",
                "wmodelMagic": "WINT",
                "wmodelSha256": "B" * 64,
                "runtimeReceiptSha256": "C" * 64,
                "usageCount": 1,
            }
        ],
    }
    expectations = tool.Expectations(
        components=2,
        instances=2,
        unique_meshes=2,
        base_meshes=1,
        supplement_meshes=1,
        base_components=1,
        base_instances=1,
        supplement_components=1,
        supplement_instances=1,
        any_negative=1,
        reflected=1,
    )
    return (
        nonstatic,
        base_static,
        base_runtime,
        supplement_runtime,
        package,
        expectations,
    )


class FoliageOverlayTests(unittest.TestCase):
    def test_native_suffix_decodes_optional_guid_blocks_and_signed_scale(self) -> None:
        suffix = native_suffix([matrix_values(-2, 3, 4, 100, 200, 300)], [1], 3)
        shadow = tool.ExportView(0, "shadowmap2d", "shadow", "world.shadow", 0, 0)
        decoded = tool.decode_native_suffix(suffix, lambda _reference: shadow, "fixture")
        self.assertEqual(decoded.version, 1)
        self.assertEqual(decoded.instance_count, 1)
        self.assertEqual(decoded.light_guid_count, 3)
        self.assertEqual(decoded.element_size, 80)
        self.assertEqual(decoded.instances[0].position, (1.0, 3.0, -2.0))
        self.assertEqual(decoded.instances[0].signed_scale, (-2.0, 4.0, 3.0))
        self.assertEqual(decoded.instances[0].quaternion, (0.0, 0.0, 0.0, 1.0))

    def test_native_suffix_rejects_wrong_element_size(self) -> None:
        suffix = bytearray(native_suffix([matrix_values(1, 1, 1, 0, 0, 0)], [1]))
        marker = len(suffix) - 80 - 8
        struct.pack_into("<I", suffix, marker, 64)
        shadow = tool.ExportView(0, "shadowmap2d", "shadow", "world.shadow", 0, 0)
        with self.assertRaisesRegex(tool.FoliageOverlayError, "element size"):
            tool.decode_native_suffix(bytes(suffix), lambda _reference: shadow, "fixture")

    def test_build_overlay_reuses_base_asset_and_defines_only_supplement(self) -> None:
        nonstatic, base, base_runtime, supplement, package, expected = fixture_documents()
        overlay = tool.build_overlay(
            nonstatic,
            base,
            base_runtime,
            supplement,
            expectations=expected,
            package_loader=lambda _source: package,
        )
        self.assertEqual([asset["assetId"] for asset in overlay["assets"]], ["SUPPLEMENT_TREE"])
        self.assertEqual(
            overlay["assets"][0]["modelPath"],
            "Map/LV_BER_BERNCASTLE/SUPPLEMENT_TREE/SUPPLEMENT_TREE.wmodel",
        )
        self.assertEqual(
            {placement["assetId"] for placement in overlay["placements"]},
            {"BASE_TREE", "SUPPLEMENT_TREE"},
        )
        self.assertEqual(overlay["summary"]["instanceCount"], 2)
        self.assertEqual(overlay["summary"]["anyNegativeScaleCount"], 1)
        self.assertEqual(overlay["summary"]["reflectedCount"], 1)
        self.assertTrue(
            all(0 < placement["placementId"] <= tool.EDITOR_ID_MASK for placement in overlay["placements"])
        )
        reflected = next(
            placement for placement in overlay["placements"] if placement["assetId"] == "SUPPLEMENT_TREE"
        )
        self.assertEqual(reflected["scale"], [-2.0, 4.0, 3.0])
        self.assertEqual(reflected["evidence"]["sourceComponentId"], "foliage-b")

    def test_low_domain_placement_collision_is_rejected(self) -> None:
        nonstatic, base, base_runtime, supplement, package, expected = fixture_documents()
        with mock.patch.object(tool, "stable_placement_id", return_value=7):
            with self.assertRaisesRegex(tool.FoliageOverlayError, "placement ID collision"):
                tool.build_overlay(
                    nonstatic,
                    base,
                    base_runtime,
                    supplement,
                    expectations=expected,
                    package_loader=lambda _source: package,
                )

    def test_atomic_write_round_trips_validated_overlay(self) -> None:
        nonstatic, base, base_runtime, supplement, package, expected = fixture_documents()
        overlay = tool.build_overlay(
            nonstatic,
            base,
            base_runtime,
            supplement,
            expectations=expected,
            package_loader=lambda _source: package,
        )
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "foliage.overlay.json"
            tool.atomic_write_overlay(output, overlay, expected)
            self.assertEqual(json.loads(output.read_text(encoding="utf-8")), overlay)
            self.assertEqual(list(output.parent.glob("*.staging")), [])


if __name__ == "__main__":
    unittest.main()
