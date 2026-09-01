import copy
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PIPELINE = ROOT / "Tools/ValtanPipeline"
if str(PIPELINE) not in sys.path:
    sys.path.insert(0, str(PIPELINE))

from validate_valtan_requested_pattern_coverage import (  # noqa: E402
    CoverageError,
    STATUS_PATTERN_CONTRACTS,
    REQUESTED_PRODUCT_IDS,
    REQUESTED_REFERENCE_ONLY_IDS,
    _index_patterns,
    _load_object,
    _validate_status_patterns,
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
        cls.gameplay = _load_object(ROOT / "Data/Valtan/Valtan.gameplay.json")
        cls.presentation = _load_object(
            ROOT / "Data/Valtan/Valtan.presentation.json"
        )
        cls.encounter = _load_object(
            ROOT / "Data/Encounters/Valtan/ValtanEncounter.json"
        )
        cls.bindings = _load_object(
            ROOT / "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
        )
        cls.effect_cues = _load_object(
            ROOT / "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
        )
        cls.promotion_manifest = _load_object(
            ROOT / "Data/Valtan/Valtan.animation-chain-promotions.json"
        )

    def test_real_owner_sets_include_derived_mechanic_patterns(self) -> None:
        self.assertEqual(self.report.product_count, len(self.report.product_ids))
        self.assertEqual(self.report.encounter_count, len(self.report.encounter_ids))
        self.assertGreater(self.report.product_count, 0)
        self.assertLess(self.report.product_count, self.report.encounter_count)

    def test_requested_existing_product_patterns_are_playable(self) -> None:
        self.assertLessEqual(set(REQUESTED_PRODUCT_IDS), self.report.product_ids)

    def test_reference_rows_are_not_product_or_scripted_sequence(self) -> None:
        for pattern_id in REQUESTED_REFERENCE_ONLY_IDS:
            with self.subTest(pattern_id=pattern_id):
                self.assertIn(pattern_id, self.report.encounter_ids)
                self.assertNotIn(pattern_id, self.report.product_ids)
                self.assertNotIn(pattern_id, self.report.scripted_pattern_ids)

    def test_derived_status_patterns_are_stable_product_rows(self) -> None:
        self.assertEqual(
            frozenset(STATUS_PATTERN_CONTRACTS),
            self.report.status_pattern_ids,
        )
        for pattern_id in STATUS_PATTERN_CONTRACTS:
            with self.subTest(pattern_id=pattern_id):
                self.assertIn(pattern_id, self.report.product_ids)
                self.assertIn(pattern_id, self.report.encounter_ids)
                self.assertNotIn(pattern_id, self.report.scripted_pattern_ids)

    def test_bind_slot_contract_tracks_saved_composition(self) -> None:
        bind = STATUS_PATTERN_CONTRACTS["VALTAN_BIND_SLOT"]
        self.assertEqual("속박 패턴", bind["displayName"])
        self.assertEqual((420623, 400442), bind["sourceActionIds"])
        self.assertEqual(
            (
                (420623, 1, "PRIMARY"),
                (400442, 0, "REFERENCE_400442_0"),
            ),
            bind["presentationSources"],
        )

    def test_bind_slot_source_and_occurrence_drift_fail_closed(self) -> None:
        def validate_status(
            gameplay: dict, presentation: dict, bindings: dict
        ) -> None:
            _validate_status_patterns(
                gameplay,
                _index_patterns(gameplay, "Valtan.gameplay"),
                _index_patterns(presentation, "Valtan.presentation"),
                _index_patterns(self.encounter, "ValtanEncounter"),
                bindings,
                self.effect_cues,
                self.promotion_manifest,
            )

        gameplay = copy.deepcopy(self.gameplay)
        bind_gameplay = next(
            row for row in gameplay["patterns"]
            if row["patternId"] == "VALTAN_BIND_SLOT"
        )
        bind_gameplay["sourceActionIds"][-1] = 400441
        with self.assertRaisesRegex(CoverageError, "source action closure"):
            validate_status(gameplay, self.presentation, self.bindings)

        presentation = copy.deepcopy(self.presentation)
        bind_presentation = next(
            row for row in presentation["patterns"]
            if row["patternId"] == "VALTAN_BIND_SLOT"
        )
        bind_presentation["stages"][0]["animation"]["occurrences"][1][
            "playMs"
        ] = 901
        with self.assertRaisesRegex(CoverageError, "selected animation differs"):
            validate_status(self.gameplay, presentation, self.bindings)

        bindings = copy.deepcopy(self.bindings)
        bind_binding = next(
            row for row in bindings["bindings"]
            if row["actionId"] == "valtan.authoring.bind-slot.step-01"
        )
        bind_binding["clips"][-1]["playMs"] = 899
        with self.assertRaisesRegex(CoverageError, "binding differs"):
            validate_status(self.gameplay, self.presentation, bindings)

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
