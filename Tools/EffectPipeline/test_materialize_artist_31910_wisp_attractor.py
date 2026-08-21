from __future__ import annotations

import copy
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest


SCRIPT_PATH = Path(__file__).with_name(
    "materialize_artist_31910_wisp_attractor.py"
)
SPEC = importlib.util.spec_from_file_location("artist_31910_wisp", SCRIPT_PATH)
assert SPEC is not None and SPEC.loader is not None
module = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(module)


class Artist31910WispAttractorTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.restored_rows = module.validate_evidence()

    def _load_document(self) -> dict:
        return json.loads(module.DOCUMENT_PATH.read_text(encoding="utf-8-sig"))

    def test_repository_document_is_materialized_and_idempotent(self) -> None:
        document = self._load_document()
        self.assertTrue(module.validate_document(document, self.restored_rows))
        self.assertFalse(module.run(write=False))
        text = module.DOCUMENT_PATH.read_text(encoding="utf-8-sig")
        self.assertEqual(
            module.build_document_text(text, document, self.restored_rows), text
        )

    def test_exact_reviewed_cohort_and_only_one_attractor(self) -> None:
        document = self._load_document()
        elements = document["elements"]
        self.assertEqual(len(elements), module.FINAL_ROW_COUNT)
        self.assertEqual(
            [row["id"] for row in elements[:7]], list(module.FULL_COHORT_IDS)
        )
        attractors = [
            row
            for row in elements
            if row.get("detail", {}).get("particle", {}).get("targetAttractor")
        ]
        self.assertEqual([row["id"] for row in attractors], [module.ATTRACTOR_ID])
        self.assertEqual(
            attractors[0]["detail"]["particle"]["targetAttractor"],
            module.ATTRACTOR_CONTRACT,
        )

    def test_restored_source_rows_match_donor_except_project_tuned_attractor(self) -> None:
        _, _, donor = module._load(module.DONOR_PATH)
        donor_by_id = module._indexed_rows(donor, label="donor")
        document_by_id = {
            row["id"]: row for row in self._load_document()["elements"]
        }
        for element_id in module.RESTORED_IDS:
            expected = copy.deepcopy(donor_by_id[element_id])
            if element_id == module.ATTRACTOR_ID:
                expected["detail"]["particle"]["targetAttractor"] = copy.deepcopy(
                    module.ATTRACTOR_CONTRACT
                )
            self.assertEqual(document_by_id[element_id], expected)

    def test_retained_forty_one_rows_and_unreviewed_decal_are_unchanged(self) -> None:
        elements = self._load_document()["elements"]
        retained = elements[len(module.RESTORED_IDS) :]
        self.assertEqual(len(retained), module.BASE_ROW_COUNT)
        self.assertEqual(
            module._canonical_sha256(retained), module.BASE_ROWS_SHA256
        )
        self.assertNotIn(
            "authored.source-decal.c474efdb506cab9df8bbce3a",
            {row["id"] for row in elements},
        )

    def test_fresh_baseline_materializes_without_reformatting_retained_tail(self) -> None:
        final_text = module.DOCUMENT_PATH.read_text(encoding="utf-8-sig")
        final_document = json.loads(final_text)
        baseline_document = copy.deepcopy(final_document)
        baseline_document["elements"] = baseline_document["elements"][
            len(module.RESTORED_IDS) :
        ]
        baseline_text = json.dumps(
            baseline_document, ensure_ascii=False, indent=2, allow_nan=False
        ) + "\n"
        rendered = module.build_document_text(
            baseline_text, baseline_document, self.restored_rows
        )
        rendered_document = json.loads(rendered)
        self.assertTrue(
            module.validate_document(rendered_document, self.restored_rows)
        )
        self.assertEqual(
            rendered_document["elements"][len(module.RESTORED_IDS) :],
            baseline_document["elements"],
        )
        baseline_start, baseline_end = module._elements_array_bounds(baseline_text)
        rendered_start, rendered_end = module._elements_array_bounds(rendered)
        baseline_array = baseline_text[baseline_start:baseline_end]
        rendered_array = rendered[rendered_start:rendered_end]
        self.assertTrue(rendered_array.endswith(baseline_array))
        self.assertTrue(rendered_array.startswith("\n    {"))

    def test_invalid_receipt_fails_before_product_document_write(self) -> None:
        final_document = self._load_document()
        baseline_document = copy.deepcopy(final_document)
        baseline_document["elements"] = baseline_document["elements"][
            len(module.RESTORED_IDS) :
        ]
        baseline_bytes = (
            json.dumps(
                baseline_document, ensure_ascii=False, indent=2, allow_nan=False
            )
            + "\n"
        ).encode("utf-8")
        receipt = json.loads(
            module.MATERIALIZATION_RECEIPT_PATH.read_text(encoding="utf-8-sig")
        )
        receipt["attractorProvenance"] = "SOURCE_EXACT"
        with tempfile.TemporaryDirectory() as temporary_directory:
            temporary_root = Path(temporary_directory)
            document_path = temporary_root / "effect.json"
            receipt_path = temporary_root / "receipt.json"
            document_path.write_bytes(baseline_bytes)
            receipt_path.write_text(
                json.dumps(receipt, ensure_ascii=False), encoding="utf-8"
            )
            with self.assertRaises(module.ArtistWispAttractorError):
                module.run(
                    write=True,
                    document_path=document_path,
                    receipt_path=receipt_path,
                )
            self.assertEqual(document_path.read_bytes(), baseline_bytes)

    def test_partial_or_retained_drift_fails_closed(self) -> None:
        partial = self._load_document()
        partial["elements"] = partial["elements"][1:]
        with self.assertRaises(module.ArtistWispAttractorError):
            module.validate_document(partial, self.restored_rows)

        drifted = self._load_document()
        drifted["elements"][-1]["displayName"] += " drift"
        with self.assertRaises(module.ArtistWispAttractorError):
            module.validate_document(drifted, self.restored_rows)

    def test_donor_role_and_track_a_evidence_drift_fail_closed(self) -> None:
        with tempfile.TemporaryDirectory() as temporary_directory:
            temporary_root = Path(temporary_directory)

            donor = module.DONOR_PATH.read_bytes()
            donor_path = temporary_root / "donor.json"
            donor_path.write_bytes(donor + b" ")
            with self.assertRaises(module.ArtistWispAttractorError):
                module.validate_evidence(donor_path=donor_path)

            role = json.loads(module.ROLE_PATH.read_text(encoding="utf-8-sig"))
            for system in role["skills"]:
                if system.get("skillId") != 31910:
                    continue
                for row in system.get("roles", []):
                    if row.get("role") == "PALE_YELLOW_RED_ORBIT":
                        row["stableIds"] = row["stableIds"][:-1]
            role_path = temporary_root / "role.json"
            role_path.write_text(
                json.dumps(role, ensure_ascii=False), encoding="utf-8"
            )
            with self.assertRaises(module.ArtistWispAttractorError):
                module.validate_evidence(role_path=role_path)

            track_a = json.loads(
                module.TRACK_A_RECEIPT_PATH.read_text(encoding="utf-8-sig")
            )

            def mutate(value: object) -> bool:
                if isinstance(value, dict):
                    if value.get("targetElementId") == module.RESTORED_IDS[0]:
                        value["sourceOrder"] += 1
                        return True
                    return any(mutate(child) for child in value.values())
                if isinstance(value, list):
                    return any(mutate(child) for child in value)
                return False

            self.assertTrue(mutate(track_a))
            track_path = temporary_root / "track-a.json"
            track_path.write_text(
                json.dumps(track_a, ensure_ascii=False), encoding="utf-8"
            )
            with self.assertRaises(module.ArtistWispAttractorError):
                module.validate_evidence(track_a_path=track_path)


if __name__ == "__main__":
    unittest.main()
