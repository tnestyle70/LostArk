#!/usr/bin/env python3

from __future__ import annotations

import importlib.util
import json
import tempfile
import unittest
from pathlib import Path


SCRIPT = Path(__file__).with_name("extract_artist_31470_v4_missing_textures.py")
SPEC = importlib.util.spec_from_file_location("artist_v4_missing_textures", SCRIPT)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class Artist31470V4MissingTextureTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.receipt = json.loads(MODULE.DEFAULT_OUTPUT.read_text(encoding="utf-8"))

    def test_receipt_and_runtime_payload_validate(self) -> None:
        MODULE.validate(self.receipt, MODULE.DEFAULT_OUTPUT)

    def test_exact_source_and_channel_contract(self) -> None:
        rows = {row["logicalTexturePath"]: row for row in self.receipt["assets"]}
        flow = rows["fx_tex_02.fx_d_fluid_032_1_cl"]
        self.assertEqual(flow["sourceOccurrenceIds"], ["source-active-013", "source-active-014"])
        self.assertEqual((flow["sourceShaderRegister"], flow["sourceShaderChannel"]), ("t3", "G"))
        impact = rows["fx_tex_03.fx_e_electric_002_cl"]
        self.assertEqual(impact["sourceOccurrenceIds"], ["source-active-027"])
        self.assertEqual((impact["sourceShaderRegister"], impact["sourceShaderChannel"]), ("t3", "A"))
        self.assertEqual(
            [row["dds"]["rawSha256"] for row in self.receipt["assets"]],
            [row["dds"]["rawSha256"] for row in MODULE.EXPECTED_ASSETS],
        )

    def test_payload_does_not_forge_program_or_product_admission(self) -> None:
        row = self.receipt["assets"][0]
        self.assertTrue(row["runtimeDeploymentAdmission"])
        self.assertFalse(row["materialProgramAdmission"])
        self.assertFalse(row["nativeVfPassAdmission"])
        self.assertFalse(row["productAdmission"])

    def test_mutated_seal_is_rejected(self) -> None:
        mutated = json.loads(json.dumps(self.receipt))
        mutated["assets"][0]["sourceShaderChannel"] = "R"
        with tempfile.TemporaryDirectory() as temporary:
            with self.assertRaisesRegex(ValueError, "receipt seal changed"):
                MODULE.validate(mutated, Path(temporary) / "receipt.json")

    def test_mutated_runtime_payload_is_rejected(self) -> None:
        mutated = json.loads(json.dumps(self.receipt))
        original = MODULE.REPO_ROOT / mutated["assets"][0]["runtimeRelativePath"]
        with tempfile.TemporaryDirectory() as temporary:
            clone = Path(temporary) / original.name
            clone.write_bytes(original.read_bytes()[:-1] + bytes([original.read_bytes()[-1] ^ 1]))
            mutated["assets"][0]["runtimeRelativePath"] = clone.as_posix()
            mutated_without_seal = dict(mutated)
            mutated_without_seal.pop("receiptSha256")
            mutated["receiptSha256"] = MODULE.canonical_sha(mutated_without_seal)
            with self.assertRaisesRegex(ValueError, "runtime DDS changed"):
                MODULE.validate(mutated, Path(temporary) / "receipt.json")


if __name__ == "__main__":
    unittest.main()
