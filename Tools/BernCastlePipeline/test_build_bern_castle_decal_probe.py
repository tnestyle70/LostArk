from __future__ import annotations

import json
import contextlib
import io
import struct
import sys
import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace


HERE = Path(__file__).resolve().parent
if str(HERE) not in sys.path:
    sys.path.insert(0, str(HERE))

import build_bern_castle_decal_probe as probe


class DecalProbeTests(unittest.TestCase):
    def fixture_document(self) -> dict:
        return {
            "areaId": probe.AREA_ID,
            "items": [
                {
                    "id": probe.FIXTURE_ID,
                    "type": "decal",
                    "references": [
                        {
                            "role": "material",
                            "objectPath": probe.INSTANCE_OBJECT_PATH,
                        }
                    ],
                }
            ],
        }

    def test_select_fixture_requires_exact_material(self) -> None:
        fixture = probe.select_fixture(self.fixture_document())
        self.assertEqual(probe.FIXTURE_ID, fixture["id"])

        bad = self.fixture_document()
        bad["items"][0]["references"][0]["objectPath"] = "wrong.material"
        with self.assertRaises(probe.ProbeError):
            probe.select_fixture(bad)

    def test_select_fixture_requires_exact_area(self) -> None:
        bad = self.fixture_document()
        bad["areaId"] = "OTHER"
        with self.assertRaises(probe.ProbeError):
            probe.select_fixture(bad)

    def test_scan_static_switch_entries(self) -> None:
        names = ["None", probe.OPACITY_SWITCH]
        guid = bytes.fromhex(probe.OPACITY_SWITCH_GUID)
        entry = struct.pack("<iiii", 1, 0, 0, 0) + guid
        serial = b"abc" + entry + b"padding" + entry
        rows = probe.scan_static_switch_entries(
            serial, names, probe.OPACITY_SWITCH, probe.OPACITY_SWITCH_GUID
        )
        self.assertEqual(2, len(rows))
        self.assertFalse(rows[0]["value"])
        self.assertFalse(rows[0]["override"])
        self.assertEqual(3, rows[0]["offsetWithinSerial"])

    def test_scan_static_switch_rejects_non_boolean(self) -> None:
        names = [probe.OPACITY_SWITCH]
        guid = bytes.fromhex(probe.OPACITY_SWITCH_GUID)
        serial = struct.pack("<iiii", 0, 0, 2, 0) + guid
        with self.assertRaises(probe.ProbeError):
            probe.scan_static_switch_entries(
                serial, names, probe.OPACITY_SWITCH, probe.OPACITY_SWITCH_GUID
            )

    def test_dds_info_reads_header(self) -> None:
        header = bytearray(128)
        header[:4] = b"DDS "
        struct.pack_into("<I", header, 4, 124)
        struct.pack_into("<II", header, 12, 32, 64)
        struct.pack_into("<I", header, 28, 7)
        header[84:88] = b"DXT5"
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "fixture.dds"
            path.write_bytes(bytes(header) + b"data")
            info = probe.dds_info(path)
        self.assertEqual(64, info["width"])
        self.assertEqual(32, info["height"])
        self.assertEqual(7, info["mipCount"])
        self.assertEqual("DXT5", info["fourCC"])

    def test_dds_info_rejects_wrong_magic(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "fixture.dds"
            path.write_bytes(b"bad")
            with self.assertRaises(probe.ProbeError):
                probe.dds_info(path)

    def test_qualified_path_distinguishes_local_and_imported(self) -> None:
        context = SimpleNamespace(
            logical_name="local",
            imports=[
                SimpleNamespace(object_name="external", package_index=0),
            ],
            exports=[
                SimpleNamespace(object_name="asset", package_index=0),
            ],
        )
        self.assertEqual("local.asset", probe.qualified_path(context, 1))
        self.assertEqual("external", probe.qualified_path(context, -1))

    def test_load_json_rejects_array_root(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "bad.json"
            path.write_text(json.dumps([]), encoding="utf-8")
            with self.assertRaises(probe.ProbeError):
                probe.load_json(path)

    def test_parse_args_is_dry_run_by_default(self) -> None:
        args = probe.parse_args(
            [
                "--nonstatic-manifest", "manifest.json",
                "--umodel", "umodel.exe",
                "--package-root", "packages",
            ]
        )
        self.assertFalse(args.write)
        self.assertIsNone(args.output_root)

    def test_write_requires_output_root(self) -> None:
        with contextlib.redirect_stderr(io.StringIO()):
            with self.assertRaises(SystemExit):
                probe.parse_args(
                    [
                        "--nonstatic-manifest", "manifest.json",
                        "--umodel", "umodel.exe",
                        "--package-root", "packages",
                        "--write",
                    ]
                )


if __name__ == "__main__":
    unittest.main()
