"""Portable contract checks; real package/decoder validation is an explicit CLI run."""

from pathlib import Path
import struct
import tempfile
import unittest

import extract_ue3_texture_mips as mips


def dds(fourcc: bytes, width: int = 8, height: int = 8, levels: int = 1) -> bytes:
    header = [0] * 31
    header[0], header[1], header[2], header[3] = 124, 0x81007, height, width
    header[6], header[18], header[19] = levels, 32, 4
    header[20], header[26] = struct.unpack("<I", fourcc)[0], 0x1000
    payload = b"".join(bytes([level + 1]) * mips.block_bytes(
        max(1, width >> level), max(1, height >> level), fourcc) for level in range(levels))
    return b"DDS " + struct.pack("<31I", *header) + payload


class TextureMipContractTests(unittest.TestCase):
    def test_bc_formats_keep_all_levels_and_small_block_tail(self):
        for fourcc in (b"DXT1", b"DXT5", b"ATI2"):
            with self.subTest(fourcc=fourcc):
                parsed = mips.parse_dds(dds(fourcc, levels=4))
                self.assertEqual(parsed.mip_count, 4)
                self.assertEqual(parsed.payloads[0], bytes([1]) * (32 if fourcc == b"DXT1" else 64))
                self.assertEqual(len(parsed.payloads[-1]), 8 if fourcc == b"DXT1" else 16)

    def test_rejects_unknown_format_cube_truncated_and_trailing_payload(self):
        good = dds(b"DXT1")
        unknown = bytearray(good)
        unknown[84:88] = b"DX10"
        cube = bytearray(good)
        struct.pack_into("<I", cube, 112, 0x200)
        for bad in (unknown, cube, good[:-1], good + b"\0"):
            with self.subTest(length=len(bad)):
                with self.assertRaises(mips.MipExtractionError):
                    mips.parse_dds(bytes(bad))

    def test_native_rotation_preserves_every_payload_and_logical_bulk_offset(self):
        records = [mips.NativeMip(0x10, 8, bytes([n]) * 8, 4, 4) for n in range(3)]
        prefix, source_offset, suffix = bytes(20), 512, b"opaque source tail"
        original = bytearray(prefix)
        struct.pack_into("<I", original, 16, len(records))
        for record in records:
            original.extend(struct.pack("<4i", record.flags, record.elements, len(record.packed),
                                        source_offset + len(original) + 16))
            original.extend(record.packed)
            original.extend(struct.pack("<2i", record.width, record.height))
        original.extend(suffix)
        rotated = mips.rotate_mips(bytes(original), 0, source_offset, records, suffix, 1)
        actual, actual_suffix = mips.parse_native_mips(rotated, 0, source_offset)
        self.assertEqual(actual, records[1:] + records[:1])
        self.assertEqual(actual_suffix, suffix)
        self.assertEqual(len(rotated), len(original))

    def test_rejects_external_bulk(self):
        serial = bytes(16) + struct.pack("<I4i", 1, 0, 8, 8, 999) + bytes(8) + struct.pack("<2i", 4, 4)
        with self.assertRaisesRegex(mips.MipExtractionError, "external/non-inline"):
            mips.parse_native_mips(serial, 0, 100)

    def test_rejects_scratch_outside_repository_out(self):
        with tempfile.TemporaryDirectory() as folder:
            directory = Path(folder).resolve()
            with self.assertRaisesRegex(mips.MipExtractionError, "repository out"):
                mips.validate_paths(directory / "source.upk", directory, directory / "umodel.exe",
                                    directory / "expected.dds", directory / "output.dds",
                                    directory / "output.json", directory / "scratch")

    def test_unsupported_input_preserves_existing_output_and_receipt(self):
        # This fails before any package/decoder use. The decoder is deliberately
        # not executable: validating DDS admission must precede launching it.
        out = Path(__file__).resolve().parents[2] / "out"
        out.mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(prefix="mip-contract-", dir=out) as folder:
            directory = Path(folder).resolve()
            source_dir, decoder_dir = directory / "source", directory / "decoder"
            source_dir.mkdir()
            decoder_dir.mkdir()
            source, decoder = source_dir / "source.upk", decoder_dir / "umodel.exe"
            source.write_bytes(b"source must stay unchanged")
            decoder.write_bytes(b"decoder must never execute")
            expected, output, receipt = directory / "input.dds", directory / "output.dds", directory / "receipt.json"
            expected.write_bytes(b"not a supported DDS")
            output.write_bytes(b"previous output")
            receipt.write_bytes(b"previous receipt")
            with self.assertRaisesRegex(mips.MipExtractionError, "legacy BC DDS"):
                mips.extract_texture_mips(source_object="package.tex.object", source_package=source,
                                         package_root=source_dir, umodel=decoder, expected_mip0=expected,
                                         output=output, receipt=receipt, scratch_root=directory / "scratch")
            self.assertEqual(output.read_bytes(), b"previous output")
            self.assertEqual(receipt.read_bytes(), b"previous receipt")
            self.assertEqual(source.read_bytes(), b"source must stay unchanged")
            self.assertFalse((directory / "scratch").exists())


if __name__ == "__main__":
    unittest.main()
