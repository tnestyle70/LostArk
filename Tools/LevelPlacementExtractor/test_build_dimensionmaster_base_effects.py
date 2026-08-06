#!/usr/bin/env python3

from __future__ import annotations

import unittest

from build_dimensionmaster_base_effects import (
    EXPECTED_SLOTS,
    PARTIAL_RUNTIME_MODULE_CLASSES,
)


class DimensionMasterBaseEffectTests(unittest.TestCase):
    def test_base_slot_contract_excludes_specialization_z(self) -> None:
        self.assertEqual(
            list(EXPECTED_SLOTS),
            ["BA", "Q", "W", "E", "R", "A", "S", "D", "F", "T", "V"],
        )
        self.assertNotIn("Z", EXPECTED_SLOTS)
        self.assertEqual(len(set(EXPECTED_SLOTS.values())), 11)

    def test_partial_module_contract_never_claims_required_as_exact(self) -> None:
        self.assertIn("particlemodulelifetime", PARTIAL_RUNTIME_MODULE_CLASSES)
        self.assertNotIn("particlemodulerequired", PARTIAL_RUNTIME_MODULE_CLASSES)


if __name__ == "__main__":
    unittest.main()
