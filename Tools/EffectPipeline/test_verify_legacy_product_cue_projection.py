#!/usr/bin/env python3

from __future__ import annotations

import json
from pathlib import PurePosixPath
import unittest

import verify_legacy_product_cue_projection as verifier


PATH = PurePosixPath("fixture.animevents")


def _document(*rows: str, version: int = 3, owner: str = "Artist") -> bytes:
    return (
        f'LOSTARK_ANIM_EVENTS {version} "{owner}" {len(rows)}\n'
        + "\n".join(rows)
        + "\n"
    ).encode("utf-8")


VALID_ROW = (
    '"clip_a" EFFECT startms=10 endms=20 payload="effect.fixture" '
    'effectref=asset anchor="root" follow=follow stop=cue_end '
    "px=0 py=1 pz=2 rx=3 ry=4 rz=5 sx=1 sy=2 sz=3"
)


class LegacyProductCueProjectionTests(unittest.TestCase):
    def test_parser_projects_the_runtime_admission_tuple_and_policies(self) -> None:
        cues = verifier._parse_product_cues(
            _document(VALID_ROW), PATH, "Artist"
        )

        self.assertEqual(1, len(cues))
        self.assertEqual("clip_a", cues[0]["clipName"])
        self.assertEqual("effect.fixture", cues[0]["effectAssetId"])
        self.assertEqual("cue_end", cues[0]["stopPolicy"])
        self.assertEqual([1.0, 2.0, 3.0], cues[0]["localTransform"]["scale"])
        self.assertRegex(cues[0]["cueId"], r"^legacy-product-cue-[0-9a-f]{64}$")

    def test_parser_ignores_non_product_effect_rows(self) -> None:
        source_row = (
            '"clip_a" EFFECT startms=10 payload="Package.Source" '
            "effectref=source"
        )

        cues = verifier._parse_product_cues(
            _document(source_row, VALID_ROW), PATH, "Artist"
        )

        self.assertEqual(1, len(cues))

    def test_parser_rejects_version_count_and_duplicate_admission(self) -> None:
        with self.assertRaisesRegex(verifier.ProjectionError, "owner/version"):
            verifier._parse_product_cues(
                _document(VALID_ROW, version=6), PATH, "Artist"
            )
        malformed_count = _document(VALID_ROW).replace(
            b' "Artist" 1\n', b' "Artist" 2\n'
        )
        with self.assertRaisesRegex(verifier.ProjectionError, "row count mismatch"):
            verifier._parse_product_cues(malformed_count, PATH, "Artist")
        with self.assertRaisesRegex(verifier.ProjectionError, "Duplicate admitted"):
            verifier._parse_product_cues(
                _document(VALID_ROW, VALID_ROW), PATH, "Artist"
            )

    def test_json_loader_rejects_duplicate_keys_and_non_finite_values(self) -> None:
        with self.assertRaisesRegex(verifier.ProjectionError, "duplicate key"):
            verifier._load_json_bytes(b'{"a":1,"a":2}\n', PATH, require_lf=True)
        with self.assertRaisesRegex(verifier.ProjectionError, "non-finite"):
            verifier._load_json_bytes(b'{"a":NaN}\n', PATH, require_lf=True)

    def test_projection_comparison_reports_mutated_row(self) -> None:
        expected = {"rows": [{"id": "a", "sha256": "1" * 64}]}
        actual = json.loads(json.dumps(expected))
        actual["rows"][0]["sha256"] = "2" * 64

        with self.assertRaisesRegex(verifier.ProjectionError, "delta is not zero"):
            verifier._assert_equal_projection(expected, actual, "mutation fixture")


if __name__ == "__main__":
    unittest.main()
