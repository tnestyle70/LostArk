#!/usr/bin/env python3
"""Regression tests for the LanceMaster V source restoration candidate."""

from __future__ import annotations

import importlib.util
import json
import unittest
from pathlib import Path

SCRIPT_PATH = Path(__file__).resolve().with_name(
    "build_lancemaster_34610_source_restoration_candidates.py")
SPEC = importlib.util.spec_from_file_location(
    "build_lancemaster_34610_source_restoration_candidates", SCRIPT_PATH)
assert SPEC is not None and SPEC.loader is not None
builder = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(builder)

REPO_ROOT = Path(__file__).resolve().parents[2]


class RestorationCandidateTest(unittest.TestCase):
    def setUp(self):
        self.document, self.imported, self.live = builder.build(REPO_ROOT)
        self.committed = json.loads(
            (REPO_ROOT / builder.OUTPUT_RELATIVE).read_text(encoding="utf-8"))

    def test_committed_candidate_is_current(self):
        self.assertEqual(self.committed, self.document)

    def test_candidate_holds_only_occurrences_the_product_lacks(self):
        live = builder.live_emitter_keys(REPO_ROOT)
        imported = json.loads(
            (REPO_ROOT / builder.IMPORTED_RELATIVE).read_text(encoding="utf-8"))
        expected = [e for e in imported["elements"]
                    if e["id"].lower() not in live]
        self.assertEqual(len(self.document["elements"]), len(expected))
        self.assertGreater(len(expected), 0)

    def test_element_ids_are_stable_and_unique(self):
        ids = [e["id"] for e in self.document["elements"]]
        self.assertEqual(len(ids), len(set(ids)))
        for element in self.document["elements"]:
            self.assertTrue(element["id"].startswith("restore.lancemaster.34610."))

    def test_carrier_is_copied_never_inferred(self):
        """A restored sprite stays a sprite and a mesh keeps its wmodel."""
        imported = {
            builder.stable_element_id(e["id"]): e
            for e in json.loads(
                (REPO_ROOT / builder.IMPORTED_RELATIVE).read_text(
                    encoding="utf-8"))["elements"]}
        checked = 0
        for element in self.document["elements"]:
            source = imported[element["id"]]
            self.assertEqual(element["kind"], source["kind"])
            self.assertEqual(
                (element.get("sourceRecipe") or {}).get("rendererShape"),
                (source.get("sourceRecipe") or {}).get("rendererShape"))
            self.assertEqual(element["material"]["renderProfile"],
                             source["material"]["renderProfile"])
            self.assertEqual(element.get("resources"), source.get("resources"))
            checked += 1
        self.assertEqual(checked, len(self.document["elements"]))

    def test_no_invented_source_material_identity(self):
        """Material identity is never guessed; unresolved stays disabled."""
        for element in self.document["elements"]:
            profile = element["material"].get("sourceProfile") or {}
            if profile.get("enabled"):
                self.assertTrue(profile.get("parentMaterialPath"))
                self.assertTrue(profile.get("profileId"))

    def test_document_is_authoring_version_13(self):
        self.assertEqual(self.document["version"], 13)
        self.assertEqual(self.document["schema"], "lostark.effect-authoring")
        for element in self.document["elements"]:
            self.assertIn("transformInheritance", element)


if __name__ == "__main__":
    unittest.main()
