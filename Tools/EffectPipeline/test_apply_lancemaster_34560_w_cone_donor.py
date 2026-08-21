from __future__ import annotations

import json
import pathlib
import tempfile
import unittest

from Tools.EffectPipeline import apply_lancemaster_34560_w_cone_donor as subject


class LanceConeDonorTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        root = pathlib.Path(self.temporary.name)
        self.donor = root / "donor.json"
        self.target = root / "target.json"
        self.donor.write_bytes(subject.DONOR_PATH.read_bytes())
        original = json.loads(subject.TARGET_PATH.read_text(encoding="utf-8-sig"))
        original["elements"] = original["elements"][:4]
        self.target.write_text(
            json.dumps(original, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def test_write_then_check_is_idempotent(self) -> None:
        self.assertTrue(
            subject.run(write=True, donor_path=self.donor, target_path=self.target)
        )
        first = self.target.read_bytes()
        self.assertFalse(
            subject.run(write=False, donor_path=self.donor, target_path=self.target)
        )
        self.assertEqual(first, self.target.read_bytes())

    def test_missing_donor_is_rejected_in_check_mode(self) -> None:
        with self.assertRaises(subject.LanceConeDonorError):
            subject.run(write=False, donor_path=self.donor, target_path=self.target)

    def test_donor_drift_is_rejected(self) -> None:
        document = json.loads(self.donor.read_text(encoding="utf-8-sig"))
        row = next(
            item for item in document["elements"] if item["id"] == subject.DONOR_ELEMENT_ID
        )
        row["detail"]["transform"]["rotationDegrees"][1] = 0
        self.donor.write_text(json.dumps(document), encoding="utf-8")
        with self.assertRaises(subject.LanceConeDonorError):
            subject.run(write=True, donor_path=self.donor, target_path=self.target)

    def test_target_baseline_drift_is_rejected_without_write(self) -> None:
        document = json.loads(self.target.read_text(encoding="utf-8-sig"))
        document["elements"][0]["visible"] = not document["elements"][0]["visible"]
        self.target.write_text(json.dumps(document), encoding="utf-8")
        before = self.target.read_bytes()
        with self.assertRaises(subject.LanceConeDonorError):
            subject.run(write=True, donor_path=self.donor, target_path=self.target)
        self.assertEqual(before, self.target.read_bytes())


if __name__ == "__main__":
    unittest.main()
