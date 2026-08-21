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
    "materialize_dimensionmaster_2050120_glasshole02_k01.py"
)
SPEC = importlib.util.spec_from_file_location("glasshole02_k01", SCRIPT_PATH)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class DimensionMasterGlasshole02K01Tests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.target = json.loads(
            (ROOT / MODULE.TARGET_RELATIVE_PATH).read_text(encoding="utf-8")
        )

    def test_exact_product_occurrence_uses_opcode_16(self) -> None:
        element = MODULE.exact_element(self.target)
        self.assertEqual(element["sourceNode"], MODULE.TARGET_SOURCE_NODE)
        self.assertEqual(element["material"]["sourceMaterialPath"], MODULE.SOURCE_MATERIAL)
        self.assertEqual(element["material"]["templateId"], "effect.standard")
        self.assertEqual(element["material"]["sourceProfile"], {"enabled": False})
        self.assertEqual(element["material"]["execution"], MODULE.expected_execution())

    def test_packet_closes_roles_channels_static_set_and_masks(self) -> None:
        execution = MODULE.expected_execution()
        self.assertEqual(
            [(row["role"], row["sourceChannel"], row["colorSpace"])
             for row in execution["textureLanes"]],
            [
                ("aura_texture", "RGBA", "srgb"),
                ("cracknormal_tex", "RG", "linear"),
                ("in_hole_texture", "RGB", "srgb"),
            ],
        )
        self.assertEqual(execution["dynamicConsumedMask"], 0x1)
        self.assertEqual(execution["dynamicSuppressedMask"], 0xE)
        self.assertEqual(execution["inputConsumedMask"], [0x77FF8CBF, 0x3])
        self.assertEqual(execution["inputSuppressedMask"], [0x88007340, 0])
        self.assertEqual(execution["staticSelectedMask"], 0x24)
        self.assertEqual(execution["staticConsumedMask"], 0x3F)
        self.assertEqual(execution["renderConsumedMask"], 0x2F)
        self.assertEqual(execution["renderSuppressedMask"], 0x10)

    def test_j_child_occurrences_are_not_promoted(self) -> None:
        targets = (
            ("effect.dimensionmaster.skill.2050210.unified.effect.json",
             "authored.source-particle.af78afd810918ca8c7f99099"),
            ("effect.dimensionmaster.skill.2050240.clip2.unified.effect.json",
             "authored.source-particle.e3b6e68de72a888afbc5fb3a"),
        )
        for filename, element_id in targets:
            with self.subTest(filename=filename):
                document = json.loads((
                    ROOT / "Data/Effects/Authored" / filename
                ).read_text(encoding="utf-8"))
                element = next(row for row in document["elements"]
                               if row["id"] == element_id)
                execution = element.get("material", {}).get("execution") or {}
                self.assertNotEqual(execution.get("opcode"), MODULE.RUNTIME_OPCODE)

    def test_native_dxbc_remains_oracle_only(self) -> None:
        MODULE.validate_native_oracle(ROOT)
        receipt = json.loads((
            ROOT / MODULE.RECEIPT_RELATIVE_PATH
        ).read_text(encoding="utf-8"))
        self.assertFalse(receipt["nativeOracle"]["runtimeAdmission"])
        self.assertEqual(receipt["runtimeExecutor"], "TYPED_HLSL_SEMANTIC_REPLAY")
        self.assertEqual(receipt["runtimeAdmission"], "AUTHORING_ONLY")
        self.assertEqual(receipt["productJoin"], "AUTHORED_NOT_PUBLISHED")
        self.assertEqual(receipt["userReview"], "PENDING")

    def test_wrong_packet_is_rejected_by_idempotence_contract(self) -> None:
        mutated = copy.deepcopy(self.target)
        element = MODULE.exact_element(mutated)
        element["material"]["execution"]["textureLanes"][1]["sourceChannel"] = "A"
        with self.assertRaisesRegex(RuntimeError, "opcode 16 packet changed"):
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
