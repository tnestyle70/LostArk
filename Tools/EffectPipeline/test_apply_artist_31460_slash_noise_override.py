from __future__ import annotations

import copy
import importlib.util
import json
import pathlib
import tempfile
import unittest


SCRIPT_PATH = pathlib.Path(__file__).with_name(
    "apply_artist_31460_slash_noise_override.py"
)
SPEC = importlib.util.spec_from_file_location("artist_a_noise_override", SCRIPT_PATH)
assert SPEC is not None and SPEC.loader is not None
module = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(module)


class ArtistASlashNoiseOverrideTests(unittest.TestCase):
    def _temporary_target(self, text: str | None = None) -> pathlib.Path:
        directory = tempfile.TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        path = pathlib.Path(directory.name) / module.TARGET_PATH.name
        if text is None:
            path.write_bytes(module.TARGET_PATH.read_bytes())
        else:
            path.write_text(text, encoding="utf-8", newline="")
        return path

    def test_repository_target_is_admitted_and_idempotent(self) -> None:
        module.run(write=False)
        current = module.TARGET_PATH.read_text(encoding="utf-8-sig")
        self.assertEqual(module.build_text(current), current)

    def test_write_changes_only_eight_target_blocks(self) -> None:
        source = module.TARGET_PATH.read_text(encoding="utf-8-sig")
        document = json.loads(source)
        for element in document["elements"]:
            if element["id"] not in module.TARGET_ELEMENT_IDS:
                continue
            scalar = next(
                row
                for row in element["material"]["sourceProfile"]["scalars"]
                if row["name"] == module.PARAMETER_NAME
            )
            scalar["value"] = module.COMPILER_VALUE
            element.pop("authoringOverrides", None)
        baseline = json.dumps(document, ensure_ascii=False, indent=2) + "\n"
        path = self._temporary_target(baseline)
        self.assertTrue(module.run(write=True, target_path=path))
        first = path.read_bytes()
        self.assertFalse(module.run(write=True, target_path=path))
        self.assertEqual(path.read_bytes(), first)
        result = json.loads(path.read_text(encoding="utf-8-sig"))
        targets = [
            row for row in result["elements"] if row["id"] in module.TARGET_ELEMENT_IDS
        ]
        self.assertEqual(len(targets), 8)
        for element in targets:
            scalar = next(
                row
                for row in element["material"]["sourceProfile"]["scalars"]
                if row["name"] == module.PARAMETER_NAME
            )
            self.assertEqual(scalar["value"], 0)
            self.assertEqual(
                element["authoringOverrides"]["scalars"],
                [
                    {
                        "name": module.PARAMETER_NAME,
                        "value": 0,
                        "compilerValue": module.COMPILER_VALUE,
                    }
                ],
            )

    def test_unknown_material_or_partial_override_fails_closed(self) -> None:
        document = json.loads(module.TARGET_PATH.read_text(encoding="utf-8-sig"))
        target = next(
            row
            for row in document["elements"]
            if row["id"] == module.TARGET_ELEMENT_IDS[0]
        )
        target["material"]["sourceMaterialPath"] = "unknown.material"
        with self.assertRaises(module.ArtistSlashOverrideError):
            module.build_text(json.dumps(document, ensure_ascii=False, indent=2))
        target["material"]["sourceMaterialPath"] = module.EXPECTED_MATERIAL_PATH
        target["authoringOverrides"]["scalars"][0]["compilerValue"] = -9
        with self.assertRaises(module.ArtistSlashOverrideError):
            module.build_text(json.dumps(document, ensure_ascii=False, indent=2))

    def test_missing_or_duplicate_stable_id_fails_closed(self) -> None:
        document = json.loads(module.TARGET_PATH.read_text(encoding="utf-8-sig"))
        missing = copy.deepcopy(document)
        missing["elements"] = [
            row for row in missing["elements"] if row["id"] != module.TARGET_ELEMENT_IDS[0]
        ]
        with self.assertRaises(module.ArtistSlashOverrideError):
            module.build_text(json.dumps(missing, ensure_ascii=False, indent=2))
        duplicate = copy.deepcopy(document)
        duplicate["elements"].append(copy.deepcopy(duplicate["elements"][0]))
        with self.assertRaises(module.ArtistSlashOverrideError):
            module.build_text(json.dumps(duplicate, ensure_ascii=False, indent=2))


if __name__ == "__main__":
    unittest.main()
