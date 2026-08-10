from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from build_artist_31470_wmodel_geometry_parity import (
    canonical_lf_tracked_json_sha256,
    generated_json_equal_after_eol_normalization,
    raw_artifact_sha256,
)


class GeometryParityHashRoleTests(unittest.TestCase):
    def test_tracked_json_hash_normalizes_only_line_endings(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            lf = root / "tracked-lf.json"
            crlf = root / "tracked-crlf.json"
            bom = root / "tracked-bom.json"
            payload = b'{\n  "count": 1\n}\n'
            lf.write_bytes(payload)
            crlf.write_bytes(payload.replace(b"\n", b"\r\n"))
            bom.write_bytes(b"\xef\xbb\xbf" + payload)

            self.assertEqual(
                canonical_lf_tracked_json_sha256(lf),
                canonical_lf_tracked_json_sha256(crlf),
            )
            self.assertNotEqual(
                canonical_lf_tracked_json_sha256(lf),
                canonical_lf_tracked_json_sha256(bom),
            )

    def test_generated_check_accepts_eol_only_and_rejects_semantic_drift(self) -> None:
        expected = b'{\n  "count": 1,\n  "status": "PINNED"\n}\n'
        self.assertTrue(
            generated_json_equal_after_eol_normalization(
                expected.replace(b"\n", b"\r\n"), expected
            )
        )
        self.assertTrue(
            generated_json_equal_after_eol_normalization(
                expected.replace(b"\n", b"\r"), expected
            )
        )
        self.assertFalse(
            generated_json_equal_after_eol_normalization(
                b'{\n  "count": 1.0,\n  "status": "PINNED"\n}\n', expected
            )
        )
        self.assertFalse(
            generated_json_equal_after_eol_normalization(
                b'{\n  "status": "PINNED",\n  "count": 1\n}\n', expected
            )
        )
        self.assertFalse(
            generated_json_equal_after_eol_normalization(
                b"\xef\xbb\xbf" + expected, expected
            )
        )

    def test_external_gltf_hash_is_byte_exact(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "carrier.gltf"
            path.write_bytes(b'{"asset":{"version":"2.0"}}\n')
            before = raw_artifact_sha256(path)
            path.write_bytes(b'{"asset":{"version":"2.1"}}\n')
            self.assertNotEqual(before, raw_artifact_sha256(path))


if __name__ == "__main__":
    unittest.main()
