from __future__ import annotations

import copy
import importlib.util
import json
import pathlib
import tempfile
import unittest


SCRIPT_PATH = pathlib.Path(__file__).with_name("apply_artist_31210_symbol_decal.py")
SPEC = importlib.util.spec_from_file_location("artist_31210_symbol", SCRIPT_PATH)
assert SPEC is not None and SPEC.loader is not None
module = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(module)


class Artist31210SymbolDecalTests(unittest.TestCase):
    def _temporary_ba4(self, document: dict) -> pathlib.Path:
        directory = tempfile.TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        path = pathlib.Path(directory.name) / module.BA4_PATH.name
        path.write_text(
            json.dumps(document, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
            newline="",
        )
        return path

    def test_repository_composition_is_admitted(self) -> None:
        module.run(write=False)

    def test_append_is_atomic_and_idempotent(self) -> None:
        document = json.loads(module.BA4_PATH.read_text(encoding="utf-8-sig"))
        document["elements"] = document["elements"][:68]
        path = self._temporary_ba4(document)
        self.assertTrue(module.run(write=True, ba4_path=path))
        first = path.read_bytes()
        self.assertFalse(module.run(write=True, ba4_path=path))
        self.assertEqual(path.read_bytes(), first)
        result = json.loads(path.read_text(encoding="utf-8-sig"))
        self.assertEqual(result["elements"][-1], module.build_symbol_decal())

    def test_tuned_row_or_decal_drift_fails_closed(self) -> None:
        document = json.loads(module.BA4_PATH.read_text(encoding="utf-8-sig"))
        baseline = copy.deepcopy(document)
        baseline["elements"][0]["visible"] = not baseline["elements"][0]["visible"]
        with self.assertRaises(module.ArtistSymbolDecalError):
            module.validate_ba4(baseline)
        decal = copy.deepcopy(document)
        decal["elements"][-1]["detail"]["decal"]["depth"] = 99
        with self.assertRaises(module.ArtistSymbolDecalError):
            module.validate_ba4(decal)

    def test_projection_scale_remains_invertible_and_centered(self) -> None:
        decal = module.build_symbol_decal()
        start = decal["detail"]["transform"]["scale"]
        end = decal["detail"]["linearLerp"]["endScale"]
        center = decal["detail"]["transform"]["position"]
        for fraction in (0.0, 0.5, 0.999999):
            scale = [a + (b - a) * fraction for a, b in zip(start, end)]
            self.assertGreater(scale[0] * scale[1] * scale[2], 1e-8)
            self.assertEqual(center, [0, 0.03, 0])
        self.assertEqual(
            decal["detail"]["transform"]["revolutionDegreesPerSecond"],
            [0, -720, 0],
        )
        self.assertEqual(
            decal["detail"]["linearLerp"]["endColorMultiply"][3], 0
        )


if __name__ == "__main__":
    unittest.main()
