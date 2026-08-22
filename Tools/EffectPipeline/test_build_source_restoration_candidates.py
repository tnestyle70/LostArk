#!/usr/bin/env python3
"""Regression tests for the Tool-only source restoration candidates."""

from __future__ import annotations

import importlib.util
import json
import unittest
from pathlib import Path

SCRIPT_PATH = Path(__file__).resolve().with_name(
    "build_source_restoration_candidates.py")
SPEC = importlib.util.spec_from_file_location(
    "build_source_restoration_candidates", SCRIPT_PATH)
assert SPEC is not None and SPEC.loader is not None
builder = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(builder)

REPO_ROOT = Path(__file__).resolve().parents[2]


class RestorationCandidateTest(unittest.TestCase):
    def _built(self, target):
        document, _, _ = builder.build(REPO_ROOT, target)
        return document

    def test_every_committed_candidate_is_current(self):
        for key, target in sorted(builder.TARGETS.items()):
            with self.subTest(target=key):
                committed = json.loads(
                    (REPO_ROOT / target.output).read_text(encoding="utf-8"))
                self.assertEqual(committed, self._built(target))

    def test_candidates_hold_only_occurrences_the_product_lacks(self):
        for key, target in sorted(builder.TARGETS.items()):
            with self.subTest(target=key):
                live = builder.live_emitter_keys(REPO_ROOT, target)
                imported = json.loads(
                    (REPO_ROOT / target.imported).read_text(encoding="utf-8"))
                expected = [e for e in imported["elements"]
                            if e["id"].lower() not in live]
                self.assertEqual(
                    len(self._built(target)["elements"]), len(expected))
                self.assertGreater(len(expected), 0)

    def test_element_ids_are_stable_and_unique(self):
        for key, target in sorted(builder.TARGETS.items()):
            with self.subTest(target=key):
                document = self._built(target)
                ids = [e["id"] for e in document["elements"]]
                self.assertEqual(len(ids), len(set(ids)))
                prefix = "restore.%s.%s." % (
                    target.character_class, target.skill_id)
                for element_id in ids:
                    self.assertTrue(element_id.startswith(prefix))

    def test_carrier_is_copied_never_inferred(self):
        """A restored sprite stays a sprite and a mesh keeps its wmodel."""
        for key, target in sorted(builder.TARGETS.items()):
            with self.subTest(target=key):
                document = self._built(target)
                imported = {
                    builder.stable_element_id(target, e["id"]): e
                    for e in json.loads(
                        (REPO_ROOT / target.imported).read_text(
                            encoding="utf-8"))["elements"]}
                for element in document["elements"]:
                    source = imported[element["id"]]
                    self.assertEqual(element["kind"], source["kind"])
                    self.assertEqual(
                        (element.get("sourceRecipe") or {}).get("rendererShape"),
                        (source.get("sourceRecipe") or {}).get("rendererShape"))
                    self.assertEqual(element["material"]["renderProfile"],
                                     source["material"]["renderProfile"])
                    self.assertEqual(element.get("resources"),
                                     source.get("resources"))

    def test_no_invented_source_material_identity(self):
        for key, target in sorted(builder.TARGETS.items()):
            with self.subTest(target=key):
                for element in self._built(target)["elements"]:
                    profile = element["material"].get("sourceProfile") or {}
                    if profile.get("enabled"):
                        self.assertTrue(profile.get("parentMaterialPath"))
                        self.assertTrue(profile.get("profileId"))

    def test_documents_are_authoring_version_13(self):
        for key, target in sorted(builder.TARGETS.items()):
            with self.subTest(target=key):
                document = self._built(target)
                self.assertEqual(document["version"], 13)
                self.assertEqual(document["schema"], "lostark.effect-authoring")
                for element in document["elements"]:
                    self.assertIn("transformInheritance", element)

    def test_candidates_are_not_catalogued(self):
        """A candidate is Tool-only until the user approves its rows."""
        catalog = json.loads(
            (REPO_ROOT / "Data/Effects/EffectCatalog.json").read_text(
                encoding="utf-8"))
        claimed = {entry["effectAssetId"] for entry in catalog["effects"]}
        for key, target in sorted(builder.TARGETS.items()):
            with self.subTest(target=key):
                self.assertNotIn(target.asset_id, claimed)


if __name__ == "__main__":
    unittest.main()
