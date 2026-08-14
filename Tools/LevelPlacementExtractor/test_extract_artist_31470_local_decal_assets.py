#!/usr/bin/env python3

from __future__ import annotations

import importlib.util
import json
import tempfile
import unittest
from pathlib import Path


SCRIPT = Path(__file__).with_name(
    "extract_artist_31470_local_decal_assets.py"
)
SPEC = importlib.util.spec_from_file_location(
    "artist_local_decal_assets", SCRIPT
)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class Artist31470LocalDecalAssetTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.receipt = json.loads(
            MODULE.DEFAULT_OUTPUT.read_text(encoding="utf-8")
        )

    def test_source_era_cdo_and_runtime_payload_validate(self) -> None:
        MODULE.validate(self.receipt)
        cdo = self.receipt["sourceEraEfgame"]["classDefaultObject"]
        self.assertEqual(cdo["exportIndex"], 9471)
        self.assertEqual(cdo["properties"]["farplane"]["value"], 300.0)
        self.assertEqual(
            cdo["properties"]["defaultsize"]["value"], {"x": 50.0, "y": 50.0}
        )
        self.assertEqual(
            cdo["properties"]["blendrange"]["value"], {"x": 100.0, "y": 100.0}
        )
        self.assertTrue(
            cdo["properties"]["bonlycalcrotationyaw"]["value"]
        )

    def test_native_register_and_channel_roles_are_pinned(self) -> None:
        rows = {
            row["logicalTexturePath"]: row
            for row in self.receipt["assets"]
        }
        height = rows["fx_tex_01.fx_c_decal_002_2"]
        dissolve = rows["fx_tex_01.fx_c_decal_002_1"]
        self.assertEqual(
            (height["sourceShaderRegister"], height["sourceShaderSampler"],
             height["sourceShaderChannel"]),
            ("t0", "s0", "B"),
        )
        self.assertEqual(
            (dissolve["sourceShaderRegister"],
             dissolve["sourceShaderSampler"],
             dissolve["sourceShaderChannel"]),
            ("t2", "s5", "G"),
        )

    def test_payload_does_not_forge_native_or_product_admission(self) -> None:
        decision = self.receipt["decision"]
        self.assertTrue(decision["boundedSemanticReplayEligible"])
        self.assertFalse(decision["nativeVfPassAdmission"])
        self.assertFalse(decision["nativeMrtAdmission"])
        self.assertFalse(decision["productAdmission"])

    def test_mutated_cdo_or_seal_is_rejected(self) -> None:
        mutated = json.loads(json.dumps(self.receipt))
        mutated["sourceEraEfgame"]["classDefaultObject"]["properties"][
            "farplane"
        ]["value"] = 301.0
        with self.assertRaisesRegex(ValueError, "receipt seal changed"):
            MODULE.validate(mutated)

    def test_mutated_runtime_payload_is_rejected(self) -> None:
        mutated = json.loads(json.dumps(self.receipt))
        original = MODULE.REPO_ROOT / mutated["assets"][0][
            "runtimeRelativePath"
        ]
        with tempfile.TemporaryDirectory() as temporary:
            clone = Path(temporary) / original.name
            raw = original.read_bytes()
            clone.write_bytes(raw[:-1] + bytes([raw[-1] ^ 1]))
            mutated["assets"][0]["runtimeRelativePath"] = clone.as_posix()
            sealed = dict(mutated)
            sealed.pop("receiptSha256")
            mutated["receiptSha256"] = MODULE.canonical_sha(sealed)
            with self.assertRaisesRegex(
                ValueError, "deployed LocalDecal DDS changed"
            ):
                MODULE.validate(mutated)


if __name__ == "__main__":
    unittest.main()
