from __future__ import annotations

import copy
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import unittest


ROOT = Path(__file__).resolve().parents[2]
SCRIPT_PATH = (
    ROOT / "Tools/EffectPipeline/"
    "materialize_dimensionmaster_2050230_fluid01_sprite.py"
)
SPEC = importlib.util.spec_from_file_location("fluid01_sprite", SCRIPT_PATH)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class DimensionMasterFluid01SpriteTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.target = json.loads(
            (ROOT / MODULE.TARGET_RELATIVE_PATH).read_text(encoding="utf-8")
        )

    def test_two_exact_product_occurrences_use_opcode_17(self) -> None:
        elements = MODULE.exact_elements(self.target)
        self.assertEqual(
            [row["id"] for row in elements], list(MODULE.TARGET_ELEMENT_IDS)
        )
        for element in elements:
            with self.subTest(element=element["id"]):
                self.assertEqual(
                    element["sourceNode"],
                    MODULE.TARGET_SOURCE_NODES[element["id"]],
                )
                self.assertEqual(
                    element["material"]["sourceMaterialPath"],
                    MODULE.SOURCE_MATERIAL,
                )
                self.assertEqual(
                    element["material"]["templateId"], "effect.standard"
                )
                self.assertEqual(
                    element["material"]["sourceProfile"], {"enabled": False}
                )
                self.assertEqual(
                    element["material"]["execution"],
                    MODULE.expected_execution(),
                )

    def test_packet_closes_four_roles_and_22_scalars(self) -> None:
        execution = MODULE.expected_execution()
        self.assertEqual(
            [
                (row["role"], row["sourceChannel"], row["colorSpace"])
                for row in execution["textureLanes"]
            ],
            [
                ("transition_texture", "RGB", "linear"),
                ("emissive_tex", "RGB", "linear"),
                ("uv_noise_01_tex", "RG", "linear"),
                ("uv_noise_02_tex", "RG", "linear"),
            ],
        )
        self.assertEqual(execution["textureMask"], 0x0F)
        self.assertEqual(execution["dynamicConsumedMask"], 0x07)
        self.assertEqual(execution["dynamicSuppressedMask"], 0x08)
        self.assertEqual(execution["inputConsumedMask"], [0x003FFFFF, 0])
        self.assertEqual(execution["scalarCount"], 22)
        self.assertEqual(execution["vectorCount"], 0)

    def test_source_evidence_and_runtime_dds_are_sealed(self) -> None:
        MODULE.validate_source_evidence(ROOT)
        hashes = MODULE.validate_runtime_resources(ROOT)
        self.assertEqual(len(hashes), 4)

    def test_all_other_dimensionmaster_f_rows_and_artist_f_are_frozen(self) -> None:
        MODULE.validate_unowned_elements(self.target)
        MODULE.validate_artist_f_golden(ROOT)

    def test_native_dxbc_is_explicitly_not_claimed(self) -> None:
        receipt = json.loads(
            (ROOT / MODULE.RECEIPT_RELATIVE_PATH).read_text(encoding="utf-8")
        )
        self.assertFalse(receipt["nativeOracle"]["selected"])
        self.assertFalse(receipt["nativeOracle"]["runtimeAdmission"])
        self.assertIsNone(receipt["nativeOracle"]["pixelShaderSha256"])
        self.assertEqual(
            receipt["runtimeExecutor"], "TYPED_SOURCE_RECONSTRUCTION"
        )
        self.assertEqual(receipt["runtimeAdmission"], "AUTHORING_ONLY")
        self.assertEqual(receipt["userReview"], "PENDING")

    def test_wrong_role_is_rejected_by_idempotence_contract(self) -> None:
        mutated = copy.deepcopy(self.target)
        first = MODULE.exact_elements(mutated)[0]
        first["material"]["execution"]["textureLanes"][3]["role"] = (
            "uv_noise_01_tex"
        )
        with self.assertRaisesRegex(RuntimeError, "opcode 17 packet changed"):
            MODULE.promote_document(mutated)

    def test_materializer_check_is_current(self) -> None:
        result = subprocess.run(
            [sys.executable, str(SCRIPT_PATH), "--mode", "check"],
            cwd=ROOT,
            text=True,
            capture_output=True,
            check=False,
        )
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)


if __name__ == "__main__":
    unittest.main()
