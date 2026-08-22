#!/usr/bin/env python3
"""Focused tests for Valtan exact pixel-program materialization."""

from __future__ import annotations

import hashlib
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import materialize_valtan_front_back_front_exact_pixel_programs as subject


class ExactPixelProgramTests(unittest.TestCase):
    def test_program_row_preserves_evidence_only_admission(self) -> None:
        payload = b"DXBC-fixture"
        sha = hashlib.sha256(payload).hexdigest()
        target = {
            "targetId": "fixture",
            "familyId": "fixture.family",
            "sourceMaterialPath": "fixture.material",
            "rendererType": "MeshParticle",
            "structuralVfPassCandidate": {
                "selectedPixelPassReference": {
                    "shaderType": "basepass",
                    "shaderIdHex": "1" * 32,
                    "vertexFactoryTypes": ["flocalvertexfactory"],
                }
            },
            "cookedPixelShader": {
                "dxbc": {"byteSize": len(payload), "sha256": sha}
            },
        }
        extracted = {
            "1" * 32: {
                "shaderType": "basepass",
                "dxbc": {"byteSize": len(payload), "sha256": sha},
                "_bytecode": payload,
            }
        }
        rows, payloads = subject.build_program_rows(
            [target], extracted, subject.REPOSITORY_ROOT / "fixture-output"
        )
        self.assertEqual(len(rows), 1)
        self.assertFalse(rows[0]["runtimeAdmission"])
        self.assertFalse(rows[0]["productAdmission"])
        self.assertEqual(next(iter(payloads.values())), payload)

    def test_program_row_rejects_dxbc_drift(self) -> None:
        payload = b"DXBC-fixture"
        target = {
            "targetId": "fixture",
            "familyId": "fixture.family",
            "sourceMaterialPath": "fixture.material",
            "rendererType": "MeshParticle",
            "structuralVfPassCandidate": {
                "selectedPixelPassReference": {
                    "shaderType": "basepass",
                    "shaderIdHex": "1" * 32,
                    "vertexFactoryTypes": ["flocalvertexfactory"],
                }
            },
            "cookedPixelShader": {
                "dxbc": {"byteSize": len(payload), "sha256": "0" * 64}
            },
        }
        extracted = {
            "1" * 32: {
                "shaderType": "basepass",
                "dxbc": {
                    "byteSize": len(payload),
                    "sha256": hashlib.sha256(payload).hexdigest(),
                },
                "_bytecode": payload,
            }
        }
        with self.assertRaisesRegex(ValueError, "differs"):
            subject.build_program_rows(
                [target], extracted, subject.REPOSITORY_ROOT / "fixture-output"
            )


if __name__ == "__main__":
    unittest.main()
