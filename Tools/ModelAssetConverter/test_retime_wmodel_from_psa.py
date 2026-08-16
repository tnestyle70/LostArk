#!/usr/bin/env python3

from __future__ import annotations

import struct
import tempfile
import unittest
from pathlib import Path

from retime_wmodel_from_psa import (
    retime_wmodel,
    read_wmodel_animation_sections,
    validate_retime_receipt,
)


class RetimeWModelTests(unittest.TestCase):
    @staticmethod
    def _write_psa(path: Path) -> None:
        entry = bytearray(168)
        entry[:4] = b"clip"
        struct.pack_into("<iiiifffiii", entry, 128, 1, 0, 0, 0, 0.0, 10.0, 30.0, 0, 0, 11)
        header = struct.pack("<20siii", b"ANIMINFO", 0, len(entry), 1)
        path.write_bytes(header + entry)

    @staticmethod
    def _write_wmodel(path: Path) -> None:
        file_header = struct.Struct("<4sHHII")
        model_header = struct.Struct("<4sIII4I")
        section_desc = struct.Struct("<IIQQ40s")
        animation = b"WANM" + struct.pack("<Iff", 1, 10.0, 24.0)
        animation_section = file_header.pack(
            b"WINT", 1, 0, 0, len(animation)
        ) + animation
        section_offset = model_header.size + section_desc.size * 2
        content = (
            model_header.pack(b"WMOD", 2, 1, 0, 0, 0, 0, 0)
            + section_desc.pack(1, 0, section_offset, 0, b"mesh")
            + section_desc.pack(
                4, 0, section_offset, len(animation_section), b"rt_clip"
            )
            + animation_section
        )
        path.write_bytes(file_header.pack(b"WINT", 1, 0, 0, len(content)) + content)

    def test_rejects_non_wmodel_input(self) -> None:
        with self.assertRaises(ValueError):
            read_wmodel_animation_sections(b"not-a-wmodel")

    def test_rejects_truncated_content(self) -> None:
        value = struct.pack("<4sHHII", b"WINT", 1, 0, 0, 32) + b"WMOD"
        with self.assertRaises(ValueError):
            read_wmodel_animation_sections(value)

    def test_retime_receipt_check_pins_source_runtime_and_rate(self) -> None:
        with tempfile.TemporaryDirectory() as temporary_directory:
            root = Path(temporary_directory)
            psa = root / "source.psa"
            wmodel = root / "runtime.wmodel"
            receipt = root / "retime.receipt.json"
            self._write_psa(psa)
            self._write_wmodel(wmodel)
            written = retime_wmodel(wmodel, psa, receipt, "rt_")

            checked = validate_retime_receipt(
                wmodel,
                psa,
                receipt,
                "rt_",
                written["sourcePsaSha256"],
                written["beforeSha256"],
                written["afterSha256"],
                30.0,
            )
            self.assertTrue(checked["receiptVerified"])
            self.assertEqual(checked["ticksPerSecond"], [30.0])

            with self.assertRaisesRegex(ValueError, "expected after identity"):
                validate_retime_receipt(
                    wmodel,
                    psa,
                    receipt,
                    "rt_",
                    expected_after_sha256="0" * 64,
                )


if __name__ == "__main__":
    unittest.main()
