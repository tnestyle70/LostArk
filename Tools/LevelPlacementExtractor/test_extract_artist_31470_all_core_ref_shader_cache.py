#!/usr/bin/env python3

from __future__ import annotations

import struct
import unittest

from extract_artist_31470_all_core_ref_shader_cache import (
    PACKED_CODE_INDEX_BITS,
    decode_packed_shader_code_slice,
)


def descriptor(word0: int, word1: int) -> bytes:
    return bytes(16) + struct.pack("<II", word0, word1)


def positions(count: int, uncompressed_size: int) -> list[dict[str, int]]:
    return [
        {
            "codeIndex": index,
            "codeHeaderLogicalOffset": 1000 + index * 16,
            "compressedLogicalOffset": 1008 + index * 16,
            "compressedByteSize": 8,
            "uncompressedByteSize": uncompressed_size,
        }
        for index in range(count)
    ]


class PackedShaderCodeSliceTests(unittest.TestCase):
    def test_5_6_bundle_is_two_descriptor_owned_slices(self) -> None:
        code_index = 21499
        code_positions = positions(code_index + 1, 2004 + 1936)
        first = decode_packed_shader_code_slice(
            descriptor(code_index, (2004 // 4) << PACKED_CODE_INDEX_BITS),
            code_positions,
        )
        second = decode_packed_shader_code_slice(
            descriptor(
                ((2004 // 4) << PACKED_CODE_INDEX_BITS) | code_index,
                (1936 // 4) << PACKED_CODE_INDEX_BITS,
            ),
            code_positions,
        )
        self.assertEqual(
            (first["codeBlobIndex"], first["sliceOffsetInUncompressedBlob"], first["sliceByteSize"]),
            (21499, 0, 2004),
        )
        self.assertEqual(
            (second["codeBlobIndex"], second["sliceOffsetInUncompressedBlob"], second["sliceByteSize"]),
            (21499, 2004, 1936),
        )

    def test_23_raw_tail_recovers_in_range_blob_and_nonzero_slice(self) -> None:
        decoded = decode_packed_shader_code_slice(
            descriptor(0x03BC06E1, 0x05D80000),
            positions(1762, 3956),
        )
        self.assertEqual(decoded["codeBlobIndex"], 1761)
        self.assertEqual(decoded["sliceOffsetInUncompressedBlob"], 956)
        self.assertEqual(decoded["sliceByteSize"], 1496)

    def test_reserved_low_bits_are_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "reserved bits"):
            decode_packed_shader_code_slice(
                descriptor(0, (64 // 4) << PACKED_CODE_INDEX_BITS | 1),
                positions(1, 64),
            )

    def test_code_blob_index_out_of_range_is_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "out of range"):
            decode_packed_shader_code_slice(
                descriptor(1, (64 // 4) << PACKED_CODE_INDEX_BITS),
                positions(1, 64),
            )

    def test_slice_outside_uncompressed_blob_is_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "exceeds its code blob"):
            decode_packed_shader_code_slice(
                descriptor(
                    (8 << PACKED_CODE_INDEX_BITS),
                    (36 << PACKED_CODE_INDEX_BITS),
                ),
                positions(1, 128),
            )


if __name__ == "__main__":
    unittest.main()
