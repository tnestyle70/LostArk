"""Signal boundaries and stable-field merge checks; never plays audio."""
import copy
import json
from pathlib import Path
import tempfile
import unittest

import numpy as np
import restore_layered_movie_audio as restore


class LayeredMovieAudioTests(unittest.TestCase):
    def setUp(self):
        self.recipe = restore.parse(restore.DEFAULT_RECIPE.read_bytes())

    def document(self):
        return {"revision": 14, "cameraTuning": {"keep": [1, 2, 3]}, "templates": [{
            "sequenceId": self.recipe["sequenceId"], "soundTracks": [dict(
                soundTrackId=s["soundTrackId"], assetId=s["originalAssetId"],
                startMs=0, durationMs=8006, volume=1) for s in self.recipe["stems"]]}]}

    def test_patch_only_assets_and_revision_preserves_current_unrelated_fields(self):
        doc = self.document()
        text = json.dumps(doc, indent=3).encode()
        patched = restore.patch_document(text, self.recipe)
        expected = copy.deepcopy(doc)
        expected["revision"] += 1
        for row, stem in zip(expected["templates"][0]["soundTracks"], self.recipe["stems"]):
            row["assetId"] = stem["restoredAssetId"]
        self.assertEqual(restore.parse(patched), expected)
        # Reversible exact byte substitutions demonstrate formatting preservation.
        restored = patched.replace(b'"revision": 15', b'"revision": 14')
        for stem in self.recipe["stems"]:
            restored = restored.replace(stem["restoredAssetId"].encode(), stem["originalAssetId"].encode())
        self.assertEqual(restored, text)
        self.assertEqual(restore.patch_document(patched, self.recipe), patched)

    def test_conflicting_timing_gain_source_pair_is_rejected(self):
        for field, value in [("startMs", 1), ("durationMs", 7900), ("volume", .75),
                             ("sourceStartMs", 1), ("loopToDuration", True), ("assetId", "Sound/other.wav")]:
            with self.subTest(field=field):
                doc = self.document()
                doc["templates"][0]["soundTracks"][0][field] = value
                with self.assertRaises(ValueError):
                    restore.patch_document(json.dumps(doc).encode(), self.recipe)
        doc = self.document()
        doc["templates"][0]["soundTracks"].append(copy.deepcopy(doc["templates"][0]["soundTracks"][0]))
        with self.assertRaises(ValueError):
            restore.patch_document(json.dumps(doc).encode(), self.recipe)

    def test_float_wave_preserves_super_full_scale_source_samples(self):
        signal = np.array([[1.23, -1.14], [0, .125], [-.625, 2.156]], dtype=np.float32)
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "float.wav"
            restore.write_float_wave(path, 44100, signal)
            rate, actual, kind = restore.read_wave(path)
        self.assertEqual((rate, kind), (44100, 3))
        np.testing.assert_array_equal(actual, signal)

    def test_lookahead_linking_release_and_no_time_shift(self):
        a = np.zeros((400, 2)); b = a.copy()
        a[100, 0] = 1.23; b[100, 0] = 1.14
        # Last-frame peak also needs protection without appended duration.
        a[-1, 1] = -1.23; b[-1, 1] = -1.14
        result, envelope = restore.linked_envelope([a, b], 1000, self.recipe["bus"])
        base = 10 ** ((-4 + 3) / 20)
        self.assertAlmostEqual(envelope[84], base)
        self.assertLess(envelope[85], base)
        self.assertEqual(envelope[85], envelope[100])
        self.assertGreater(envelope[101], envelope[100])
        self.assertLess(envelope[101], base)
        np.testing.assert_array_equal(np.flatnonzero(np.any(result[0] != 0, axis=1)), [100, 399])
        self.assertEqual(result[0].shape, a.shape)
        self.assertLess(np.max(abs(result[0] + result[1])), 1)
        np.testing.assert_allclose(result[0], a * envelope[:, None], atol=0, rtol=0)
        np.testing.assert_allclose(result[1], b * envelope[:, None], atol=0, rtol=0)

    def test_silence_empty_and_mismatched_grid(self):
        silent = np.zeros((23, 2))
        result, envelope = restore.linked_envelope([silent, silent], 44100, self.recipe["bus"])
        self.assertFalse(np.any(result))
        self.assertTrue(np.isfinite(envelope).all())
        for stems in [[], [silent, silent[:2]], [silent[:0], silent[:0]]]:
            with self.assertRaises(ValueError):
                restore.linked_envelope(stems, 44100, self.recipe["bus"])

    def test_all_recipes_preserve_authored_box_count_and_timing(self):
        for path in Path(restore.__file__).parent.glob('*_selection_bus.recipe.json'):
            recipe = restore.parse(path.read_bytes())
            rows = [dict(soundTrackId=s['soundTrackId'], assetId=s['originalAssetId'],
                         **recipe['expectedTrack']) for s in recipe['stems']]
            doc = dict(revision=1, templates=[dict(sequenceId=recipe['sequenceId'], soundTracks=rows)])
            with self.subTest(recipe=recipe['id']):
                new = restore.parse(restore.patch_document(json.dumps(doc).encode(), recipe))
                actual = new['templates'][0]['soundTracks']
                self.assertEqual(len(actual), len(rows))
                for before, after in zip(rows, actual):
                    self.assertEqual({k:v for k,v in before.items() if k!='assetId'},
                                     {k:v for k,v in after.items() if k!='assetId'})


if __name__ == "__main__":
    unittest.main()
