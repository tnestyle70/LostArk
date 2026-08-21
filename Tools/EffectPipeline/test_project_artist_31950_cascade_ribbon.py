from __future__ import annotations

import copy
import importlib.util
import json
import tempfile
import unittest
from pathlib import Path


SCRIPT_PATH = Path(__file__).with_name(
    "project_artist_31950_cascade_ribbon.py"
)
SPEC = importlib.util.spec_from_file_location("artist_31950_ribbon", SCRIPT_PATH)
assert SPEC is not None and SPEC.loader is not None
module = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(module)


class Artist31950CascadeRibbonProjectionTests(unittest.TestCase):
    def test_repository_projection_and_receipt_are_current(self) -> None:
        module.run(write=False)

    def test_one_row_projection_is_atomic_and_idempotent(self) -> None:
        repository_document = module.read_json(module.DOCUMENT_PATH)
        baseline = copy.deepcopy(repository_document)
        target = module.target_row(baseline)
        target["visible"] = False
        target["kind"] = "particle"
        target["sourceRecipe"]["rendererShape"] = "sprite"
        target["detail"]["trail"].pop("tilingDistanceWorldUnits", None)
        target["detail"]["trail"].pop(
            "distanceTessellationStepWorldUnits", None
        )
        target["resources"] = []
        target["material"]["templateId"] = "effect.source_material"
        target["material"]["sourceProfile"] = copy.deepcopy(
            module.BASELINE_SOURCE_PROFILE
        )
        target["material"]["execution"] = {
            "enabled": False,
            "failClosed": True,
        }
        self.assertEqual(
            module.canonical_sha256(target), module.BASELINE_TARGET_SHA256
        )
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        root = Path(temporary.name)
        document_path = root / module.DOCUMENT_PATH.name
        receipt_path = root / module.OUTPUT_RECEIPT_PATH.name
        document_path.write_bytes(module.output_bytes(baseline))
        self.assertTrue(
            module.run(True, document_path, receipt_path)
        )
        first_document = document_path.read_bytes()
        first_receipt = receipt_path.read_bytes()
        self.assertFalse(
            module.run(True, document_path, receipt_path)
        )
        self.assertEqual(document_path.read_bytes(), first_document)
        self.assertEqual(receipt_path.read_bytes(), first_receipt)

    def test_projection_preserves_child_material_and_every_other_row(self) -> None:
        document = module.read_json(module.DOCUMENT_PATH)
        target = module.target_row(document)
        baseline = copy.deepcopy(document)
        baseline_target = module.target_row(baseline)
        baseline_target["visible"] = False
        baseline_target["kind"] = "particle"
        baseline_target["sourceRecipe"]["rendererShape"] = "sprite"
        baseline_target["detail"]["trail"].pop(
            "tilingDistanceWorldUnits", None
        )
        baseline_target["detail"]["trail"].pop(
            "distanceTessellationStepWorldUnits", None
        )
        baseline_target["resources"] = []
        baseline_target["material"]["templateId"] = "effect.source_material"
        baseline_target["material"]["sourceProfile"] = copy.deepcopy(
            module.BASELINE_SOURCE_PROFILE
        )
        baseline_target["material"]["execution"] = {
            "enabled": False,
            "failClosed": True,
        }
        projected = module.project_document(baseline)
        projected_target = module.target_row(projected)
        self.assertEqual(
            projected_target["material"]["sourceMaterialPath"],
            target["material"]["sourceMaterialPath"],
        )
        self.assertEqual(
            projected_target["material"]["templateId"], "effect.standard"
        )
        self.assertEqual(
            projected_target["material"]["sourceProfile"],
            {"enabled": False},
        )
        self.assertEqual(
            projected_target["resources"], module.projected_resources()
        )
        self.assertEqual(
            projected_target["material"]["execution"],
            module.projected_execution(),
        )
        self.assertEqual(
            module.canonical_sha256(
                [
                    row
                    for row in projected["elements"]
                    if row["id"] != module.TARGET_ELEMENT_ID
                ]
            ),
            module.NON_TARGET_ELEMENTS_SHA256,
        )
        self.assertEqual(projected_target["kind"], "trail")
        self.assertEqual(
            projected_target["sourceRecipe"]["rendererShape"], "ribbon"
        )

    def test_drift_and_material_overclaim_fail_closed(self) -> None:
        document = module.read_json(module.DOCUMENT_PATH)
        drift = copy.deepcopy(document)
        drift["elements"][0]["visible"] = not drift["elements"][0][
            "visible"
        ]
        with self.assertRaises(module.Artist31950RibbonProjectionError):
            module.validate_boundary(drift)

        receipt = module.read_json(module.MATERIAL_RECEIPT_PATH)
        overclaim = copy.deepcopy(receipt)
        overclaim["runtimeAdmission"]["admitted"] = True
        with self.assertRaises(module.Artist31950RibbonProjectionError):
            module.validate_material_receipt(overclaim)


if __name__ == "__main__":
    unittest.main()
