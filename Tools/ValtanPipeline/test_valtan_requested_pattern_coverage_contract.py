import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PIPELINE = ROOT / "Tools/ValtanPipeline"
if str(PIPELINE) not in sys.path:
    sys.path.insert(0, str(PIPELINE))

from validate_valtan_requested_pattern_coverage import (  # noqa: E402
    ANIMATIONLESS_SKELETON_CONTRACTS,
    REQUESTED_PRODUCT_IDS,
    REQUESTED_REFERENCE_ONLY_IDS,
    validate,
)


NATIVE_CONTRACT = (
    ROOT
    / "Tools/ValtanPatternAuditionServiceHarness/Private/ValtanCanonicalGraphContractTests.cpp"
)
HARNESS_PROJECT = (
    ROOT
    / "Tools/ValtanPatternAuditionServiceHarness/Default/ValtanPatternAuditionServiceHarness.vcxproj"
)


class ValtanRequestedPatternCoverageContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.report = validate(ROOT)
        cls.native_contract = NATIVE_CONTRACT.read_text(encoding="utf-8-sig")
        cls.harness_project = HARNESS_PROJECT.read_text(encoding="utf-8-sig")

    def test_real_owner_sets_include_three_animationless_skeletons(self) -> None:
        self.assertEqual(self.report.product_count, len(self.report.product_ids))
        self.assertEqual(self.report.encounter_count, len(self.report.encounter_ids))
        self.assertGreater(self.report.product_count, 0)
        self.assertLess(self.report.product_count, self.report.encounter_count)

    def test_requested_existing_product_patterns_are_playable(self) -> None:
        self.assertLessEqual(set(REQUESTED_PRODUCT_IDS), self.report.product_ids)

    def test_reference_rows_are_not_product_or_saved_flow(self) -> None:
        for pattern_id in REQUESTED_REFERENCE_ONLY_IDS:
            with self.subTest(pattern_id=pattern_id):
                self.assertIn(pattern_id, self.report.encounter_ids)
                self.assertNotIn(pattern_id, self.report.product_ids)
                self.assertNotIn(pattern_id, self.report.flow_pattern_ids)

    def test_animationless_authoring_skeletons_are_stable_product_rows(self) -> None:
        self.assertEqual(
            frozenset(ANIMATIONLESS_SKELETON_CONTRACTS),
            self.report.animationless_skeleton_ids,
        )
        for pattern_id in ANIMATIONLESS_SKELETON_CONTRACTS:
            with self.subTest(pattern_id=pattern_id):
                self.assertIn(pattern_id, self.report.product_ids)
                self.assertIn(pattern_id, self.report.encounter_ids)
                self.assertNotIn(pattern_id, self.report.flow_pattern_ids)

    def test_native_contract_executes_real_playable_and_next_inventory(self) -> None:
        self.assertIn(
            "ValtanCanonicalGraphContractTests.cpp", self.harness_project
        )
        for marker in (
            "CValtanPatternTree::Load(View, Status)",
            "CValtanPatternTree::Build_PlayablePatternInventory(",
            "CValtanPatternTree::Build_NextPatternInventory(",
            "Inventory.Contains(Slot.strPatternId)",
            "ManagedPatternIds.size() == Inventory.Get_PatternCount()",
            "ManagedPatternIds.size() + ReferencePatternIds.size() == Patterns.size()",
            "REQUESTED_REFERENCE_ONLY_PATTERN_IDS",
        ):
            self.assertIn(marker, self.native_contract)
        for pattern_id in REQUESTED_PRODUCT_IDS + REQUESTED_REFERENCE_ONLY_IDS:
            self.assertIn(f'"{pattern_id}"', self.native_contract)


if __name__ == "__main__":
    unittest.main()
