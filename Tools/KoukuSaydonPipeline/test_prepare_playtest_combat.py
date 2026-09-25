"""Regression checks for the detached playtest damage candidate."""
import copy
import json
from pathlib import Path
import unittest

from prepare_playtest_combat import prepare, prepare_zero_floor_madness, PREFIX, ROOT
from project_kouku_saydon_composition import validate_document, CompositionError


class PlaytestCombatCandidateTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.source = json.loads((ROOT / "Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json").read_text(encoding="utf-8-sig"))
        cls.candidate, cls.receipt = prepare(cls.source)
        cls.patterns = {p["patternId"]: p for p in cls.candidate["patterns"]}
        cls.logics = {r["logicId"]: r for r in cls.candidate["logics"]}

    def test_detached_candidate_validates_and_preserves_unrelated_tuning(self):
        before = copy.deepcopy(self.source)
        validate_document(self.candidate)
        self.assertEqual(before, self.source)
        patterns = {p["patternId"]: p for p in before["patterns"]}
        for number in (38, 85, 99):
            old = patterns[PREFIX + str(number)]
            new = self.patterns[old["patternId"]]
            by_id = {p["occurrenceId"]: p for p in new["presentationOccurrences"]}
            for occurrence in old["presentationOccurrences"]:
                current = by_id[occurrence["occurrenceId"]]
                for field in ("positionOffset", "rotationDegrees", "scale", "anchorKind", "worldOccurrenceId"):
                    self.assertEqual(occurrence.get(field), current.get(field), (occurrence["occurrenceId"], field))

    def test_every_card_has_its_own_suit_and_ninety_percent_damage(self):
        suits = set()
        for ordinal in range(125, 129):
            card = self.logics[f"kakulsaydon.g1.logic.{ordinal}"]
            self.assertEqual(1, len(card["cardSymbols"]))
            suits.update(card["cardSymbols"])
            self.assertTrue(all(h["damagePercent"] == 90 for h in card["projectileHits"]))
        self.assertEqual({"HEART", "SPADE", "CLUB", "DIAMOND"}, suits)

    def test_both_lasers_admit_contacts_for_the_visible_shot_without_reaction_gate(self):
        p = self.patterns[PREFIX + "85"]
        presentations = {o["occurrenceId"]: o for o in p["presentationOccurrences"]}
        windows = {o["occurrenceId"]: o for o in p["logicOccurrences"]}
        for beam, box in ((1, 23), (6, 24)):
            visual = presentations[f'{p["patternId"]}.presentation.{beam}']
            collider = presentations[f'{p["patternId"]}.presentation.{box}']
            window = windows[collider["logicOccurrenceId"]]
            for field in ("startMs", "durationMs"):
                self.assertEqual(visual[field], collider[field])
                self.assertEqual(visual[field], window[field])
            self.assertFalse(self.logics[window["logicId"]].get("repeatAfterKnockback", False))
            self.assertEqual(12, self.logics[window["onSuccessLogicIds"][0]]["pushRangeM"])

    def test_breath_keeps_damage_and_three_explicit_gauge_ticks(self):
        rows = [r for r in self.receipt["changes"] if r["kind"] == "breath-three-ticks"]
        self.assertEqual(8, len(rows))
        for row in rows:
            p = self.patterns[row["occurrenceId"].split(".logic.")[0]]
            window = next(o for o in p["logicOccurrences"] if o["occurrenceId"] == row["occurrenceId"])
            damage, madness = [self.logics[i] for i in window["onSuccessLogicIds"]]
            self.assertEqual(100, damage["damageAmount"])
            self.assertEqual(1, madness["percent"])
            # Server rounds each tick interval upward to its 30 Hz clock.
            interval = (self.logics[window["logicId"]]["repeatIntervalMs"] * 30 + 999) // 1000
            begin = (window["startMs"] * 30 + 999) // 1000
            end = ((window["startMs"] + window["durationMs"]) * 30 + 999) // 1000
            self.assertEqual(3, len(range(begin, end, interval)))

    def test_three_breath_rays_share_one_player_tick_budget(self):
        p = self.patterns[PREFIX + "102"]
        primary = p["patternId"] + ".logic.9"
        old = {p["patternId"] + ".logic.10", p["patternId"] + ".logic.11"}
        self.assertTrue(all(not o["enabled"] for o in p["logicOccurrences"] if o["occurrenceId"] in old))
        self.assertEqual(3, sum(o.get("logicOccurrenceId") == primary for o in p["presentationOccurrences"]))
        self.assertFalse(any(o.get("logicOccurrenceId") in old for o in p["presentationOccurrences"]))

    def test_lingering_flames_keep_damage_and_explicitly_suppress_extra_madness(self):
        before = copy.deepcopy(self.candidate)
        candidate, receipt = prepare_zero_floor_madness(before)
        validate_document(candidate)
        self.assertEqual(before, self.candidate)
        logics = {r["logicId"]: r for r in candidate["logics"]}
        old_patterns = {p["patternId"]: p for p in before["patterns"]}
        fields = [r for r in receipt["changes"] if r["kind"] == "field"]
        already_present = any(r.get("displayName") == "Breath lingering flame: no additional madness"
                              for r in before["logics"])
        self.assertEqual(0 if already_present else 21, len(fields))
        for p in candidate["patterns"]:
            if p["patternId"] not in {PREFIX + str(n) for n in (43, 102, 114)}:
                self.assertEqual(old_patterns[p["patternId"]], p)
                continue
            original = {r["occurrenceId"]: r for r in old_patterns[p["patternId"]]["logicOccurrences"]}
            for occurrence in p["logicOccurrences"]:
                if int(occurrence["occurrenceId"].rsplit(".", 1)[1]) not in range(2, 9):
                    self.assertEqual(original[occurrence["occurrenceId"]], occurrence)
                    continue
                damage, madness = [logics[i] for i in occurrence["onSuccessLogicIds"]]
                self.assertEqual(("FIXED_DAMAGE", 100), (damage["outcomeKind"], damage["damageAmount"]))
                self.assertEqual(("MADNESS_GAUGE_ADD_PERCENT", 0), (madness["outcomeKind"], madness["percent"]))
                unchanged = copy.deepcopy(occurrence)
                unchanged["onSuccessLogicIds"] = original[occurrence["occurrenceId"]]["onSuccessLogicIds"]
                self.assertEqual(original[occurrence["occurrenceId"]], unchanged)
        repeated, second = prepare_zero_floor_madness(candidate)
        self.assertEqual(candidate, repeated)
        self.assertFalse(second["changes"])
        invalid = copy.deepcopy(candidate)
        result = next(r for r in invalid["logics"] if r.get("outcomeKind") == "MAX_HP_PERCENT_DAMAGE")
        result["percent"] = 0
        with self.assertRaises(CompositionError):
            validate_document(invalid)

    def test_new_static_hits_and_dynamic_targets_use_same_push_contract(self):
        for number in (39, 120):
            p = self.patterns[PREFIX + str(number)]
            sources = {o["occurrenceId"]: o for o in p["presentationOccurrences"]}
            resources = {r["resourceId"]: r for r in self.candidate["presentationResources"]}
            windows = {o["occurrenceId"]: o for o in p["logicOccurrences"]}
            for change in self.receipt["changes"]:
                if change.get("patternId") != p["patternId"] or change["kind"] != "attack-contact":
                    continue
                asset = resources[sources[change["source"]]["resourceId"]].get("assetId", "")
                window = windows[sources[change["collider"]]["logicOccurrenceId"]]
                result = self.logics[window["onSuccessLogicIds"][0]]
                self.assertEqual(10, result["percent"])
                if "bluecircle" in asset or "fourfan" in asset:
                    self.assertTrue(result["forcePush"] and result["pushBallistic"])
                    self.assertGreater(result["pushRangeM"], 0)
            for o in p["logicOccurrences"]:
                logic = self.logics[o["logicId"]]
                if logic.get("triggerKind", logic.get("judgementKind")) == "ALBION_BLUE_CIRCLE":
                    self.assertTrue(all(h["forcePush"] and h["riseHeightM"] == 2 for h in logic["fixedHits"]))


if __name__ == "__main__":
    unittest.main()
