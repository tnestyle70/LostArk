import copy
import unittest
from Tools.ValtanPipeline import build_valtan_subtitle_candidates as source


class ValtanSubtitleCandidatesTests(unittest.TestCase):
    def fixture(self):
        document = dict(revision=1, instances=[], templates=[])
        scenes, messages = {}, {}
        for scene, track, suffix in source.SCENES:
            balloon = suffix.startswith("gate1-")
            identity = "cin.37053_" + track
            messages[identity] = "원본 자막"
            scenes.setdefault(scene, dict(rows={}))['rows'][track] = dict(
                name="source." + track, cls="efinterptracksubtitleballoon" if balloon else "efinterptracksubtitle",
                p={"subtitleballooninfoarr" if balloon else "subtitleinfoarr": [dict(strmsgid=identity, time=1.25, duration=2.5)]})
            sequence = "sequence." + suffix
            document["templates"].append(dict(sequenceId=sequence, durationMs=5000))
            document["instances"].append(dict(instanceId="world.sequence.instance.valtan.source-preview." + suffix,
                templateId=sequence, startDelayMs=0, playbackSpeed=1,
                bindings=[dict(slotId="actor", targetKind="OBJECT_RESOURCE", targetId="original.actor")]))
        return document, scenes, messages

    def test_original_actor_clock_and_positions(self):
        before, scenes, messages = self.fixture()
        result, rows = source.prepare(before, scenes, messages)
        self.assertEqual(5, len(rows))
        self.assertEqual(2, result["revision"])
        self.assertEqual(before["instances"], result["instances"])
        for row in rows:
            self.assertEqual((1250, 2500), (row["row"]["startMs"], row["row"]["durationMs"]))
            self.assertEqual("BALLOON" if "gate1-" in row["instanceId"] else "NORMAL", row["row"]["position"])

    def test_saved_row_timing_is_preserved(self):
        before, scenes, messages = self.fixture()
        saved, _ = source.prepare(before, scenes, messages)
        saved["templates"][0]["subtitleTracks"][0]["startMs"] = 1500
        result, rows = source.prepare(saved, scenes, messages)
        self.assertEqual(saved, result)
        self.assertFalse(rows)

    def test_wrong_actor_clock_and_unsupported_text_fail(self):
        before, scenes, messages = self.fixture()
        for mutate in (lambda d: d["instances"][0].update(playbackSpeed=2),
                       lambda d: d["instances"][-1]["bindings"][0].update(targetKind="MAP_PLACEMENT"),
                       lambda d: d["templates"][0].update(durationMs=2000)):
            changed = copy.deepcopy(before)
            mutate(changed)
            with self.assertRaises(ValueError):
                source.prepare(changed, scenes, messages)
        for text in ("", "<b>대사</b>", "한" * 1366, "bad\x1b"):
            changed = {k: text for k in messages}
            with self.assertRaises(ValueError):
                source.prepare(before, scenes, changed)


if __name__ == "__main__":
    unittest.main()
