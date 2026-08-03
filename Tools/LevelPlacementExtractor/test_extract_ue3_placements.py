import struct
import unittest

from extract_ue3_placements import Reader


class ReaderFStringTests(unittest.TestCase):
    def test_utf16_fstring_removes_only_the_wide_terminator(self) -> None:
        value = "Actor"
        encoded = value.encode("utf-16-le") + b"\0\0"
        reader = Reader(struct.pack("<i", -(len(value) + 1)) + encoded)

        self.assertEqual(value, reader.fstring())
        self.assertEqual(4 + len(encoded), reader.offset)

    def test_utf16_fstring_preserves_non_ascii_code_units(self) -> None:
        value = "맵"
        encoded = value.encode("utf-16-le") + b"\0\0"
        reader = Reader(struct.pack("<i", -2) + encoded)

        self.assertEqual(value, reader.fstring())


if __name__ == "__main__":
    unittest.main()
