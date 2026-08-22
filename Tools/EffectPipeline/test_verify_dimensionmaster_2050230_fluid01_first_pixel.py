from __future__ import annotations

import importlib.util
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[2]
PATH = ROOT / "Tools/EffectPipeline/verify_dimensionmaster_2050230_fluid01_first_pixel.py"
SPEC = importlib.util.spec_from_file_location("fluid01_first_pixel", PATH)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class DimensionMasterFluid01FirstPixelTests(unittest.TestCase):
    def test_product_join_and_rt0_witness(self) -> None:
        result = MODULE.run()
        self.assertEqual(result["status"], "PASS")
        self.assertEqual(result["rawFamilyRows"], 2)
        self.assertEqual(result["productRows"], 2)
        self.assertEqual(
            result["carrierDispositions"],
            ["KEEP", "KEEP"],
        )
        self.assertEqual(result["cohortRoles"], ["CANARY", "DATA_ONLY_EXPANSION"])
        self.assertTrue(all(value > 0 for value in result["rt0NonzeroGridPixels"].values()))
        self.assertEqual(result["visualStatus"], "PENDING_USER_REVIEW")


if __name__ == "__main__":
    unittest.main()
