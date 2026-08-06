#!/usr/bin/env python3

from __future__ import annotations

import struct
import unittest

from retime_wmodel_from_psa import read_wmodel_animation_sections


class RetimeWModelTests(unittest.TestCase):
    def test_rejects_non_wmodel_input(self) -> None:
        with self.assertRaises(ValueError):
            read_wmodel_animation_sections(b"not-a-wmodel")

    def test_rejects_truncated_content(self) -> None:
        value = struct.pack("<4sHHII", b"WINT", 1, 0, 0, 32) + b"WMOD"
        with self.assertRaises(ValueError):
            read_wmodel_animation_sections(value)


if __name__ == "__main__":
    unittest.main()
