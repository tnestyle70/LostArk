#!/usr/bin/env python3

from __future__ import annotations

import json
import struct
import tempfile
import unittest
from pathlib import Path

from retime_wmodel_from_psa import (
    retime_wmodel,
    read_wmodel_animation_sections,
    validate_retime_receipt,
)

FILE_HEADER = struct.Struct("<4sHHII")
MODEL_HEADER = struct.Struct("<4sIII4I")
SECTION_DESC = struct.Struct("<IIQQ40s")
PSA_CHUNK = struct.Struct("<20siii")


def build_wmodel(clips: list[tuple[str, float, float]]) -> bytes:
    """Minimal WModel holding only animation sections.

    clips is (sectionName, durationTicks, ticksPerSecond) in runtime order.
    """
    payloads = []
    for _name, duration, ticks in clips:
        body = b"WANM" + struct.pack("<Iff", 0, duration, ticks) + b"\0" * 16
        payloads.append(
            FILE_HEADER.pack(b"WINT", 1, 0, 0, len(body)) + body
        )

    section_count = len(clips)
    table_size = SECTION_DESC.size * section_count
    offset = MODEL_HEADER.size + table_size
    table = b""
    blobs = b""
    for index, (name, _duration, _ticks) in enumerate(clips):
        blob = payloads[index]
        table += SECTION_DESC.pack(
            4, index, offset, len(blob), name.encode("ascii")[:40]
        )
        blobs += blob
        offset += len(blob)

    content = (
        MODEL_HEADER.pack(b"WMOD", section_count, section_count, 0, 0, 0, 0, 0)
        + table
        + blobs
    )
    return FILE_HEADER.pack(b"WINT", 1, 0, 0, len(content)) + content


def build_psa(sequences: list[tuple[str, int, float]]) -> bytes:
    """Minimal PSA holding only an ANIMINFO chunk.

    sequences is (name, rawFrameCount, animRate).
    """
    records = b""
    first = 0
    for name, frames, rate in sequences:
        record = name.encode("ascii").ljust(64, b"\0") + b"\0" * 64
        record += struct.pack(
            "<iiiifffiii",
            1, 0, 0, 0, 0.0, float(frames), float(rate), 0, first, frames,
        )
        records += record
        first += frames
    header = PSA_CHUNK.pack(b"ANIMINFO", 0, 168, len(sequences))
    return header + records


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


class RetimeMultiSourceTests(unittest.TestCase):
    """The AnimSet path may assemble one WModel from several PSA files."""

    def setUp(self) -> None:
        self._temp = tempfile.TemporaryDirectory()
        self.root = Path(self._temp.name)
        self.addCleanup(self._temp.cleanup)

    def _write(self, name: str, blob: bytes) -> Path:
        path = self.root / name
        path.write_bytes(blob)
        return path

    def test_single_psa_still_retimes_every_clip(self) -> None:
        wmodel = self._write("one.wmodel", build_wmodel([
            ("mesh_alpha", 9.0, 30.0),
            ("mesh_beta", 4.0, 30.0),
        ]))
        psa = self._write("one.psa", build_psa([
            ("alpha", 10, 28.5),
            ("beta", 5, 24.0),
        ]))
        receipt_path = self.root / "one.receipt.json"

        receipt = retime_wmodel(wmodel, [psa], receipt_path, "mesh_")

        self.assertEqual(receipt["animationCount"], 2)
        self.assertEqual(receipt["changedAnimationCount"], 2)
        self.assertEqual(len(receipt["sourcePsaFiles"]), 1)
        rates = {
            row["name"]: row["ticksPerSecond"]
            for row in read_wmodel_animation_sections(wmodel.read_bytes())
        }
        self.assertAlmostEqual(rates["mesh_alpha"], 28.5, places=5)
        self.assertAlmostEqual(rates["mesh_beta"], 24.0, places=5)
        self.assertTrue(receipt_path.is_file())
        self.assertEqual(
            json.loads(receipt_path.read_text(encoding="utf-8"))["animationCount"], 2
        )

    def test_two_psa_files_merge_into_one_animation_set(self) -> None:
        wmodel = self._write("two.wmodel", build_wmodel([
            ("mesh_alpha", 9.0, 30.0),
            ("mesh_zulu", 4.0, 30.0),
        ]))
        first = self._write("first.psa", build_psa([("alpha", 10, 28.5)]))
        second = self._write("second.psa", build_psa([("zulu", 5, 24.0)]))

        receipt = retime_wmodel(
            wmodel, [first, second], self.root / "two.receipt.json", "mesh_"
        )

        self.assertEqual(receipt["animationCount"], 2)
        self.assertEqual(receipt["changedAnimationCount"], 2)
        self.assertEqual(len(receipt["sourcePsaFiles"]), 2)
        self.assertEqual([row["index"] for row in receipt["clips"]], [0, 1])
        self.assertEqual(
            {Path(row["sourcePsa"]).name for row in receipt["clips"]},
            {"first.psa", "second.psa"},
        )

    def test_rejects_clip_name_collision_between_psa_files(self) -> None:
        wmodel = self._write("dup.wmodel", build_wmodel([
            ("mesh_alpha", 9.0, 30.0),
            ("mesh_alpha", 9.0, 30.0),
        ]))
        first = self._write("dup_a.psa", build_psa([("alpha", 10, 28.5)]))
        second = self._write("dup_b.psa", build_psa([("alpha", 10, 24.0)]))

        with self.assertRaises(ValueError):
            retime_wmodel(
                wmodel, [first, second], self.root / "dup.receipt.json", "mesh_"
            )

    def test_rejects_frame_span_mismatch(self) -> None:
        wmodel = self._write("span.wmodel", build_wmodel([
            ("mesh_alpha", 99.0, 30.0),
        ]))
        psa = self._write("span.psa", build_psa([("alpha", 10, 28.5)]))

        with self.assertRaises(ValueError):
            retime_wmodel(
                wmodel, [psa], self.root / "span.receipt.json", "mesh_"
            )


if __name__ == "__main__":
    unittest.main()
