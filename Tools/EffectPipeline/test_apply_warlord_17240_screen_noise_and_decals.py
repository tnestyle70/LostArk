from __future__ import annotations

import copy
import importlib.util
import json
import os
import pathlib
import tempfile
import unittest


SCRIPT_PATH = pathlib.Path(__file__).with_name(
    "apply_warlord_17240_screen_noise_and_decals.py"
)
SPEC = importlib.util.spec_from_file_location("warlord_17240_correction", SCRIPT_PATH)
assert SPEC is not None and SPEC.loader is not None
module = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(module)


class Warlord17240CorrectionTests(unittest.TestCase):
    def _write_document(self, directory: pathlib.Path, name: str, document: dict) -> pathlib.Path:
        path = directory / name
        path.write_text(
            json.dumps(document, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
            newline="",
        )
        return path

    def _temporary_pre_correction(self) -> tuple[tempfile.TemporaryDirectory, pathlib.Path, pathlib.Path]:
        directory = tempfile.TemporaryDirectory()
        root = pathlib.Path(directory.name)
        ba1 = json.loads(module.BA1_SOURCE_PATH.read_text(encoding="utf-8-sig"))
        ba3 = json.loads(module.BA3_PATH.read_text(encoding="utf-8-sig"))
        ba3["elements"] = ba3["elements"][:8]
        ba1_path = self._write_document(root, module.BA1_PATH.name, ba1)
        ba3_path = self._write_document(root, module.BA3_PATH.name, ba3)
        return directory, ba1_path, ba3_path

    def test_repository_correction_is_admitted(self) -> None:
        self.assertFalse(module.run(write=False))

    def test_selective_materialization_is_idempotent(self) -> None:
        directory, ba1_path, ba3_path = self._temporary_pre_correction()
        self.addCleanup(directory.cleanup)
        original_ba1 = json.loads(ba1_path.read_text(encoding="utf-8"))
        original_ba3 = json.loads(ba3_path.read_text(encoding="utf-8"))

        self.assertTrue(module.run(write=True, ba1_path=ba1_path, ba3_path=ba3_path))
        first_ba1 = ba1_path.read_bytes()
        first_ba3 = ba3_path.read_bytes()
        self.assertFalse(module.run(write=True, ba1_path=ba1_path, ba3_path=ba3_path))
        self.assertEqual(ba1_path.read_bytes(), first_ba1)
        self.assertEqual(ba3_path.read_bytes(), first_ba3)

        result_ba1 = json.loads(ba1_path.read_text(encoding="utf-8"))
        result_ba3 = json.loads(ba3_path.read_text(encoding="utf-8"))
        _, decals = module.load_source_evidence()
        self.assertEqual(result_ba1["elements"], original_ba1["elements"][:24])
        self.assertEqual(result_ba3["elements"][:8], original_ba3["elements"])
        self.assertEqual(result_ba3["elements"][8:], decals)

    def test_track_a_receipt_or_target_drift_fails_closed(self) -> None:
        receipt = json.loads(module.TRACK_A_RECEIPT_PATH.read_text(encoding="utf-8-sig"))
        target = next(
            row
            for row in receipt["targets"]
            if row.get("targetEffectAssetId") == module.BA3_EFFECT_ID
        )
        directory = tempfile.TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        receipt_path = self._write_document(
            pathlib.Path(directory.name),
            "receipt.json",
            {"targets": [copy.deepcopy(target)]},
        )
        module.load_source_evidence(receipt_path=receipt_path)

        changed_receipt = {"targets": [copy.deepcopy(target)]}
        changed_receipt["targets"][0]["sourceDecalRows"][0]["targetBindings"][0][
            "assetId"
        ] = "Effect/Warlord/Textures/wrong.dds"
        receipt_path = self._write_document(
            pathlib.Path(directory.name), "changed-receipt.json", changed_receipt
        )
        with self.assertRaises(module.Warlord17240CorrectionError):
            module.load_source_evidence(receipt_path=receipt_path)

        _, _, ba1 = module._load_text(module.BA1_PATH)
        source_screen_post, decals = module.load_source_evidence()
        ba1_drift = copy.deepcopy(ba1)
        ba1_drift["elements"][0]["visible"] = not ba1_drift["elements"][0]["visible"]
        with self.assertRaises(module.Warlord17240CorrectionError):
            module.validate_ba1(ba1_drift, source_screen_post)
        ba3 = json.loads(module.BA3_PATH.read_text(encoding="utf-8-sig"))
        ba3_drift = copy.deepcopy(ba3)
        ba3_drift["elements"][0]["visible"] = not ba3_drift["elements"][0]["visible"]
        with self.assertRaises(module.Warlord17240CorrectionError):
            module.validate_ba3(ba3_drift, decals)
        partial = copy.deepcopy(ba3)
        partial["elements"] = partial["elements"][:10]
        with self.assertRaises(module.Warlord17240CorrectionError):
            module.validate_ba3(partial, decals)

    def test_exact_decal_lane_timing_and_recipe_contract(self) -> None:
        _, decals = module.load_source_evidence()
        self.assertEqual([row["id"] for row in decals], list(module.DECAL_IDS))
        for index, row in enumerate(decals):
            self.assertEqual(row["resources"], module.EXPECTED_RESOURCES)
            self.assertEqual(row["detail"]["timing"]["startDelaySeconds"], 0.1200000000000001)
            self.assertEqual(row["detail"]["timing"]["lifeTimeSeconds"], 2.0)
            self.assertEqual(row["detail"]["decal"], {"size": [4.5, 4.5], "depth": 0.25})
            self.assertEqual(row["sourceRecipe"]["rendererShape"], "decal")
            self.assertEqual(
                module._canonical_sha256(row["sourceRecipe"]),
                module.EXPECTED_NORMALIZED_RECIPE_SHA256[module.DECAL_IDS[index]],
            )

    def test_two_file_commit_rolls_back_on_second_replace_failure(self) -> None:
        directory, ba1_path, ba3_path = self._temporary_pre_correction()
        self.addCleanup(directory.cleanup)
        before_ba1 = ba1_path.read_bytes()
        before_ba3 = ba3_path.read_bytes()
        call_count = 0

        def fail_second(source: os.PathLike[str], target: os.PathLike[str]) -> None:
            nonlocal call_count
            call_count += 1
            if call_count == 2:
                raise OSError("injected second replace failure")
            os.replace(source, target)

        with self.assertRaises(OSError):
            module.run(
                write=True,
                ba1_path=ba1_path,
                ba3_path=ba3_path,
                replace=fail_second,
            )
        self.assertEqual(ba1_path.read_bytes(), before_ba1)
        self.assertEqual(ba3_path.read_bytes(), before_ba3)


if __name__ == "__main__":
    unittest.main()
