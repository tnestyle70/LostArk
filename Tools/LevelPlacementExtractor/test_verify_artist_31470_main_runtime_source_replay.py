#!/usr/bin/env python3

from __future__ import annotations

import copy
import json
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import verify_artist_31470_main_runtime_source_replay as replay


class MainRuntimeSourceReplayTests(unittest.TestCase):
    def test_canonical_receipt_validates(self) -> None:
        receipt = json.loads(replay.DEFAULT_OUTPUT.read_text(encoding="utf-8-sig"))
        replay.validate_receipt(receipt)

    def test_product_overclaim_is_rejected(self) -> None:
        receipt = json.loads(replay.DEFAULT_OUTPUT.read_text(encoding="utf-8-sig"))
        poisoned = copy.deepcopy(receipt)
        poisoned["decision"]["productAdmission"] = True
        replay.seal(poisoned)
        with self.assertRaisesRegex(RuntimeError, "overclaims"):
            replay.validate_receipt(poisoned)

    def test_external_opacity_overclaim_is_rejected(self) -> None:
        receipt = json.loads(replay.DEFAULT_OUTPUT.read_text(encoding="utf-8-sig"))
        poisoned = copy.deepcopy(receipt)
        poisoned["decision"]["sourceExactExternalOpacityAdmission"] = True
        replay.seal(poisoned)
        with self.assertRaisesRegex(RuntimeError, "overclaims"):
            replay.validate_receipt(poisoned)

    def test_missing_constant_case_is_rejected(self) -> None:
        receipt = json.loads(replay.DEFAULT_OUTPUT.read_text(encoding="utf-8-sig"))
        poisoned = copy.deepcopy(receipt)
        poisoned["constantTextureMutationContract"]["cases"].pop()
        replay.seal(poisoned)
        with self.assertRaisesRegex(RuntimeError, "case order"):
            replay.validate_receipt(poisoned)

    def test_spatial_result_mutation_is_rejected(self) -> None:
        receipt = json.loads(replay.DEFAULT_OUTPUT.read_text(encoding="utf-8-sig"))
        poisoned = copy.deepcopy(receipt)
        poisoned["nonuniformPointClampTranslationContract"]["cases"][0][
            "actualRuntimeRgba"
        ][0] += 0.25
        replay.seal(poisoned)
        with self.assertRaisesRegex(RuntimeError, "spatial numeric projection"):
            replay.validate_receipt(poisoned)

    def test_coordinated_spatial_result_mutation_is_rejected(self) -> None:
        receipt = json.loads(replay.DEFAULT_OUTPUT.read_text(encoding="utf-8-sig"))
        poisoned = copy.deepcopy(receipt)
        row = poisoned["nonuniformPointClampTranslationContract"]["cases"][0]
        row["originalRawDxbcRgba"][0] += 0.25
        row["actualRuntimeRgba"][0] += 0.25
        replay.seal(poisoned)
        with self.assertRaisesRegex(RuntimeError, "spatial numeric projection"):
            replay.validate_receipt(poisoned)

    def test_original_prerequisite_seal_mutation_is_rejected(self) -> None:
        receipt = json.loads(replay.DEFAULT_OUTPUT.read_text(encoding="utf-8-sig"))
        poisoned = copy.deepcopy(receipt)
        poisoned["originalDxbcReplayPrerequisite"]["receiptSha256"] = "0" * 64
        replay.seal(poisoned)
        with self.assertRaisesRegex(RuntimeError, "prerequisite seal"):
            replay.validate_receipt(poisoned)

    def test_boundary_overclaim_is_rejected(self) -> None:
        receipt = json.loads(replay.DEFAULT_OUTPUT.read_text(encoding="utf-8-sig"))
        poisoned = copy.deepcopy(receipt)
        poisoned["boundary"]["sourceExactSamplerPolicyAdmission"] = True
        replay.seal(poisoned)
        with self.assertRaisesRegex(RuntimeError, "boundary changed"):
            replay.validate_receipt(poisoned)

    def test_constant_original_provenance_mutation_is_rejected(self) -> None:
        receipt = json.loads(replay.DEFAULT_OUTPUT.read_text(encoding="utf-8-sig"))
        poisoned = copy.deepcopy(receipt)
        poisoned["constantTextureMutationContract"]["cases"][0][
            "originalShaderIdHex"
        ] = "0" * 32
        replay.seal(poisoned)
        with self.assertRaisesRegex(RuntimeError, "shader provenance"):
            replay.validate_receipt(poisoned)


if __name__ == "__main__":
    unittest.main()
