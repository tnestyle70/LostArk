import copy
import unittest
from Tools.KoukuSaydonPipeline import build_scene_subtitle_candidates as source
from Tools.KoukuSaydonPipeline import project_kouku_saydon_composition as product


class SceneSubtitleTests(unittest.TestCase):
    def resource(self, text="렛츠, 쇼타임~!\n원본 자막"):
        return dict(resourceId="subtitle.kouku.cin.37081_32_01", displayName="자막",
                    kind="SUBTITLE", assetId="cin.37081_32_01", resourceKind="",
                    defaultAnchorKind="MAP", subtitleText=text, subtitlePosition="UPPER", durationMs=2300)

    def validate(self, row):
        return product._validate_presentation_resources(dict(presentationResources=[row]))

    def test_utf8_byte_boundary_and_product_projection(self):
        row = self.resource("한" * 1365 + "x")
        self.assertEqual(4096, len(row["subtitleText"].encode("utf8")))
        self.validate(row)
        # The actual row projector must carry resource text, position and timing.
        box = dict(occurrenceId="p.presentation.1", resourceId=row["resourceId"], startMs=689, durationMs=2300)
        result = product._project_presentation_occurrence({}, {}, box, row)
        self.assertEqual(row["subtitleText"], result["subtitleText"])
        self.assertEqual("UPPER", result["subtitlePosition"])
        self.assertEqual((689, 2300), (result["startMs"], result["durationMs"]))

    def test_subtitle_screen_layout_survives_product_projection(self):
        row = self.resource()
        resources = self.validate(row)
        box = dict(occurrenceId="P1.presentation.1", resourceId=row["resourceId"], startMs=689,
                   durationMs=2300, anchorKind="MAP", followBoss=False,
                   positionOffset=[12, 60, 0], scale=[2, 2, 2])
        pattern = dict(patternId="P1", nextPresentationOccurrenceOrdinal=2, presentationOccurrences=[box])
        original = copy.deepcopy(pattern)
        product._validate_presentation_occurrences(pattern, resources, 5000, {})
        projected = product._project_presentation_occurrence({}, pattern, box, row)
        self.assertEqual([12, 60, 0], projected["positionOffset"])
        self.assertEqual([2, 2, 2], projected["scale"])
        self.assertEqual(row["subtitleText"], projected["subtitleText"])
        self.assertEqual(original, pattern)
        for patch in ({"positionOffset": [0, float("nan"), 0]}, {"scale": [0, 2, 2]}):
            with self.subTest(patch=patch), self.assertRaises(product.CompositionError):
                product._validate_presentation_occurrences(
                    {**pattern, "presentationOccurrences": [{**box, **patch}]}, resources, 5000, {})
        self.assertEqual(original, pattern)

    def test_subtitle_occurrence_anchor_and_owner_lifetime(self):
        row = self.resource()
        resources = self.validate(row)
        box = dict(occurrenceId="P1.presentation.1", resourceId=row["resourceId"], startMs=689,
                   durationMs=2300, anchorKind="MAP", followBoss=False)
        pattern = dict(patternId="P1", nextPresentationOccurrenceOrdinal=2, presentationOccurrences=[box])
        product._validate_presentation_occurrences(pattern, resources, 5000, {})
        for patch in ({"anchorKind": "BOSS"}, {"followBoss": True}, {"bone": "Bip01"},
                      {"worldId": "world.1"}, {"durationMs": 5000}):
            with self.subTest(patch=patch), self.assertRaises(product.CompositionError):
                changed = {**pattern, "presentationOccurrences": [{**box, **patch}]}
                product._validate_presentation_occurrences(changed, resources, 5000, {})

    def test_invalid_text_position_and_cross_kind_are_rejected(self):
        for text in ("", "한" * 1366, "<b>자막</b>", "bad\x1b", "\ud800", 12):
            with self.subTest(text=repr(text)), self.assertRaises(product.CompositionError):
                self.validate(self.resource(text))
        for patch in ({"subtitlePosition": "BOTTOM"}, {"kind": "CAMERA"}, {"defaultAnchorKind": "BOSS"}, {"resourceKind": "GROUP"}):
            with self.subTest(patch=patch), self.assertRaises(product.CompositionError):
                self.validate({**self.resource(), **patch})

    def test_plain_source_markup_keeps_lines_and_rejects_unknown_tags(self):
        self.assertEqual("장르를 바꾸는 거야.\n로맨스?", source.plain_text("장르를 바꾸는 거야.<br><FONT color='#6b6b6b'>로맨스?</FONT>"))
        with self.assertRaises(ValueError):
            source.plain_text("<img src='missing'>")

    def test_source_windows_use_existing_camera_clock(self):
        self.assertEqual(-15710, source.clock_ms(1.0, "gate3"))
        self.assertEqual(12258, source.clock_ms(0, "gate1"))
        self.assertEqual(58810, source.clock_ms(41.488, "gate1"))
        self.assertEqual(689, source.clock_ms(.689048707485199, "full"))

    def test_source_join_is_idempotent_and_preserves_saved_timing(self):
        scenes = {}
        for _, name, track, _ in source.SCENES:
            scenes.setdefault(name, {"rows": {}})["rows"][str(track)] = dict(cls="efinterptracksubtitle", p={"subtitleinfoarr": []})
        scenes["SCENE02B"]["rows"]["16"]["p"].update(positiontype="cinematic_subtitle_position_type_upper",
            subtitleinfoarr=[dict(strmsgid="cin.37081_32_01", time=.689, duration=2.3)])
        camera = dict(resourceId="camera", kind="CAMERA", assetId="kouku.gate3.showtime.camera.1")
        document = dict(revision=1, presentationResources=[camera], patterns=[dict(patternId="P1", durationMs=5000,
            presentationOccurrences=[dict(occurrenceId="P1.presentation.1",resourceId="camera")],nextPresentationOccurrenceOrdinal=2)])
        candidate, bindings, _ = source.prepare(document, scenes, {"cin.37081_32_01": "렛츠, 쇼타임~!"})
        self.assertEqual(1, len(bindings))
        self.assertEqual(document["patterns"][0]["presentationOccurrences"], candidate["patterns"][0]["presentationOccurrences"][:1])
        saved = copy.deepcopy(candidate)
        saved["patterns"][0]["presentationOccurrences"][-1]["startMs"] = 800
        repeated, bindings, _ = source.prepare(saved, scenes, {"cin.37081_32_01": "렛츠, 쇼타임~!"})
        self.assertEqual(saved, repeated)
        self.assertFalse(bindings)


if __name__ == "__main__":
    unittest.main()
