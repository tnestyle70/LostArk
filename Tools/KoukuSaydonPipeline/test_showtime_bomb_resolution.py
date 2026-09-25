"""Tracking-owned V1 bombs and fixed Effect birth anchors use product projection."""
import copy
import unittest

from Tools.KoukuSaydonPipeline import project_kouku_saydon_composition as p


def fixture():
    pattern_id = "KAKULSAYDON_G1_PATTERN_35"
    rows, resources = [], []
    for index, name in enumerate(("body", "explosion", "sector"), 1):
        resources.append(dict(p.PRESENTATION_RESOURCE_DEFAULTS, resourceId=name, displayName=name,
                              kind="EFFECT", assetId="effect.test." + name, resourceKind="V1_EFFECT"))
        rows.append(dict(p.PRESENTATION_OCCURRENCE_DEFAULTS,
                         occurrenceId=f"{pattern_id}.presentation.{index}", resourceId=name,
                         startMs=(100, 4000, 1000)[index-1], durationMs=4000,
                         anchorKind="BOSS" if name == "sector" else "MAP", followBoss=name == "sector",
                         positionOffset=[0, 0, 0] if name == "sector" else [5, 1.3, 942],
                         rotationDegrees=[0, 90 if name == "sector" else 17, 0],
                         scale=[2.2, 1, 1.25] if name == "sector" else [1, 1, 1]))
    logic = dict(logicId="kakulsaydon.g1.logic.1", displayName="tracking bomb", logicType="DURATION",
                 judgementKind="BOSS_TRACK_TARGET", bombPresentationOccurrenceId=rows[0]["occurrenceId"],
                 bombExplosionPresentationOccurrenceId=rows[1]["occurrenceId"],
                 bombSectorPresentationOccurrenceId=rows[2]["occurrenceId"],
                 bombSectorRadiusM=11, bombSectorHalfAngleDegrees=22.5)
    pattern = dict(patternId=pattern_id, presentationOccurrences=rows, nextPresentationOccurrenceOrdinal=4,
                   logicOccurrences=[dict(occurrenceId=pattern_id+".logic.1", logicId=logic["logicId"], startMs=1000, durationMs=2000)])
    return dict(logics=[logic], presentationResources=resources), pattern


class ShowtimeBombResolutionTests(unittest.TestCase):
    def test_projection_keeps_source_and_bakes_end_clock_and_elliptic_fan(self):
        document, pattern = fixture()
        before = copy.deepcopy((document, pattern))
        p._validate_logic_definition(document["logics"][0], "fixture", 2)
        rows, visuals, controlled = p._project_track_bombs(document, pattern)
        self.assertEqual(before, (document, pattern))
        self.assertEqual(1, len(rows))
        row = rows[0]
        self.assertEqual([5, 1.3, 942], row["position"])
        self.assertAlmostEqual(24.2, row["fan"]["radiusXM"])
        self.assertEqual(13.75, row["fan"]["radiusZM"])
        self.assertEqual(90, row["fan"]["yawDegrees"])
        self.assertEqual(22.5, row["fan"]["halfAngleDegrees"])
        body = visuals[row["bodyVisualId"]]
        self.assertEqual(2900, body["durationMs"])
        self.assertEqual([0, 0, 0], body["occurrences"][0]["positionOffset"])
        self.assertEqual([0, 17, 0], body["occurrences"][0]["rotationDegrees"])
        self.assertEqual(4000, visuals[row["explosionVisualId"]]["durationMs"])
        self.assertEqual({r["occurrenceId"] for r in pattern["presentationOccurrences"][:2]}, controlled)

    def test_invalid_references_motion_geometry_and_ownership_are_rejected(self):
        changes = [
            lambda d, s: d["logics"][0].update(bombPresentationOccurrenceId="missing"),
            lambda d, s: d["logics"][0].update(followSpeedScale=1),
            lambda d, s: d["logics"][0].update(bombSectorRadiusM=0),
            lambda d, s: d["logics"][0].update(bombSectorHalfAngleDegrees=180),
            lambda d, s: s["presentationOccurrences"][1].update(positionOffset=[1, 2, 3]),
            lambda d, s: s["presentationOccurrences"][2].update(followBoss=False),
            lambda d, s: s["presentationOccurrences"][2].update(durationMs=100),
            lambda d, s: s["logicOccurrences"].append(dict(s["logicOccurrences"][0], occurrenceId="duplicate.owner")),
            lambda d, s: s["logicOccurrences"][0].update(cancelAtEnd=True),
        ]
        for change in changes:
            document, pattern = fixture()
            change(document, pattern)
            with self.subTest(change=change), self.assertRaises(p.CompositionError):
                p._project_track_bombs(document, pattern)

    def test_partial_or_other_kind_definition_fails_and_disabled_owner_still_validates(self):
        document, pattern = fixture()
        document["logics"][0].pop("bombSectorRadiusM")
        with self.assertRaises(p.CompositionError):
            p._validate_logic_definition(document["logics"][0], "fixture", 2)
        document, pattern = fixture()
        document["logics"][0]["judgementKind"] = "AREA_OVERLAP"
        with self.assertRaises(p.CompositionError):
            p._validate_logic_definition(document["logics"][0], "fixture", 2)
        document, pattern = fixture()
        pattern["logicOccurrences"][0]["enabled"] = False
        self.assertEqual(([], {}, set()), p._project_track_bombs(document, pattern))
        pattern["presentationOccurrences"].pop()
        with self.assertRaises(p.CompositionError):
            p._project_track_bombs(document, pattern)

    def test_fixed_effect_pair_shares_birth_but_preserves_source_basis_and_rejects_chains(self):
        document, pattern = fixture()
        pattern["presentationOccurrences"].pop()
        warning, explosion = pattern["presentationOccurrences"]
        for row in (warning, explosion):
            row.update(anchorKind="BOSS", followBoss=False)
        warning["rotationDegrees"] = [0, 0, 0]
        explosion["rotationDegrees"] = [0, 180, 0]
        explosion["anchorPresentationOccurrenceId"] = warning["occurrenceId"]
        resources = {r["resourceId"]: r for r in document["presentationResources"]}
        p._validate_presentation_occurrences(pattern, resources, 10000, {})
        projected = p._project_presentation_occurrence(document, pattern, explosion, resources["explosion"])
        self.assertEqual(warning["occurrenceId"], projected["anchorPresentationOccurrenceId"])
        self.assertEqual([0, 180, 0], projected["rotationDegrees"])
        warning["anchorPresentationOccurrenceId"] = explosion["occurrenceId"]
        with self.assertRaises(p.CompositionError):
            p._validate_presentation_occurrences(pattern, resources, 10000, {})


if __name__ == "__main__":
    unittest.main()
