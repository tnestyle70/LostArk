from __future__ import annotations

import copy
import importlib.util
import json
import pathlib
import unittest


SCRIPT_PATH = pathlib.Path(__file__).with_name("verify_artist_31490_product_join.py")
SPEC = importlib.util.spec_from_file_location("artist_31490_join", SCRIPT_PATH)
assert SPEC is not None and SPEC.loader is not None
module = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(module)


class Artist31490ProductJoinTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.bindings = module.load_json(module.SKILL_BINDINGS_PATH)
        cls.events = module.ANIMEVENTS_PATH.read_text(encoding="utf-8-sig")
        cls.catalog = module.load_json(module.CATALOG_PATH)
        cls.authored = module.load_json(module.AUTHORED_PATH)

    def test_repository_authoring_join_is_closed(self) -> None:
        module.validate_documents(
            self.bindings, self.events, self.catalog, self.authored
        )

    def test_missing_or_duplicate_cue_fails_closed(self) -> None:
        lines = self.events.splitlines()
        body = [line for line in lines[1:] if line != module.EXPECTED_CUE]
        missing = f'LOSTARK_ANIM_EVENTS 5 "Artist" {len(body)}\n' + "\n".join(body)
        with self.assertRaises(module.ArtistJoinError):
            module.validate_documents(
                self.bindings, missing, self.catalog, self.authored
            )
        duplicate_body = lines[1:] + [module.EXPECTED_CUE]
        duplicate = (
            f'LOSTARK_ANIM_EVENTS 5 "Artist" {len(duplicate_body)}\n'
            + "\n".join(duplicate_body)
        )
        with self.assertRaises(module.ArtistJoinError):
            module.validate_documents(
                self.bindings, duplicate, self.catalog, self.authored
            )

    def test_wrong_catalog_or_authored_cardinality_fails_closed(self) -> None:
        catalog = copy.deepcopy(self.catalog)
        target = next(
            row for row in catalog["effects"] if row["effectAssetId"] == module.EFFECT_ID
        )
        target["payloadKind"] = "UNKNOWN"
        with self.assertRaises(module.ArtistJoinError):
            module.validate_documents(
                self.bindings, self.events, catalog, self.authored
            )
        authored = copy.deepcopy(self.authored)
        authored["elements"].pop()
        with self.assertRaises(module.ArtistJoinError):
            module.validate_documents(
                self.bindings, self.events, self.catalog, authored
            )

    def test_runtime_membership_is_unique(self) -> None:
        self.assertFalse(module.runtime_has_target({"effects": []}))
        row = {"effectAssetId": module.EFFECT_ID}
        self.assertTrue(module.runtime_has_target({"effects": [row]}))
        with self.assertRaises(module.ArtistJoinError):
            module.runtime_has_target({"effects": [row, copy.deepcopy(row)]})


if __name__ == "__main__":
    unittest.main()
