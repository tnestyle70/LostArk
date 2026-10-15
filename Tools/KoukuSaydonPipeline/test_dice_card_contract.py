"""Per-visual Dice damage metadata uses the canonical projection and row emitter."""
import copy
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Tools/KoukuSaydonPipeline"))
import project_kouku_saydon_composition as projector
import prepare_kouku_draft_play as draft

PATTERN_ID = "KOUKUSAYDON_G1_PATTERN_78"


class DiceCardContractTests(unittest.TestCase):
    def setUp(self):
        self.source = json.loads((ROOT / projector.SOURCE_PATH).read_text(encoding="utf-8-sig"))
        pattern = next(row for row in self.source["patterns"] if row["patternId"] == PATTERN_ID)
        logics = {row["logicId"]: row for row in self.source["logics"]}
        # 실제 활성 occurrence를 통해 찾는다. 미사용 과거 logic을 수정한 검사는
        # 제품 projection이 그 값을 읽는다는 근거가 될 수 없다.
        self.sources = [(row, logics[row["logicId"]]) for row in pattern["logicOccurrences"]
                        if row.get("enabled", True)
                        and logics[row["logicId"]].get("triggerKind") == "PURSUIT_PROJECTILES"]
        self.assertEqual([f"koukusaydon.g1.logic.{i}" for i in range(125, 129)],
                         [logic["logicId"] for _, logic in self.sources])
        self.assertEqual([[symbol] for symbol in ("HEART", "SPADE", "CLUB", "DIAMOND")],
                         [logic["cardSymbols"] for _, logic in self.sources])

    def project(self, source):
        _, artifacts = draft.prepare(source, ROOT, pattern_id=PATTERN_ID)
        encounter = json.loads(artifacts[Path("encounter.json")])
        pattern = next(row for row in encounter["patterns"] if row["patternId"] == PATTERN_ID)
        return pattern["pursuitProjectiles"], artifacts

    def test_symbol_contract_rejects_bad_values_and_preserves_optional_generic_pursuit(self):
        # 현재 카드마다 visual이 하나다. 배열 개수와 symbol 검증을 따로 유지한다.
        for _, logic in self.sources:
            for bad in ([], ["HEART", "SPADE"], ["NONE"], ["red"], [[]]):
                with self.subTest(logic=logic["logicId"], bad=bad), self.assertRaises(projector.CompositionError):
                    projector._pursuit_fields({**logic, "cardSymbols": bad}, "Dice")
            self.assertEqual(logic["cardSymbols"], projector._pursuit_fields(logic, "Dice")["cardSymbols"])
            generic = copy.deepcopy(logic)
            generic.pop("cardSymbols")
            self.assertNotIn("cardSymbols", projector._pursuit_fields(generic, "Generic pursuit"))

    def test_selected_projection_keeps_visual_order_color_and_source_immutable(self):
        # 각 occurrence의 독립 값을 바꿔 읽기 경로를 검증하며 디스크 정본은 건드리지 않는다.
        for index, (_, logic) in enumerate(self.sources):
            logic["projectileHits"][0]["damagePercent"] = 50 + index
        before = copy.deepcopy(self.source)
        rows, _ = self.project(self.source)
        self.assertEqual(before, self.source)
        self.assertEqual([row["occurrenceId"] for row, _ in self.sources],
                         [row["occurrenceId"] for row in rows])
        for (occurrence, logic), row in zip(self.sources, rows):
            with self.subTest(occurrence=occurrence["occurrenceId"]):
                self.assertEqual((occurrence["startMs"], occurrence["durationMs"]),
                                 (row["startMs"], row["durationMs"]))
                self.assertEqual(logic["cardSymbols"], row["cardSymbols"])
                self.assertEqual(len(logic["visualIds"]), len(row["visualIds"]))
                self.assertEqual(logic["projectileHits"][0]["damagePercent"],
                                 row["projectileHits"][0]["damagePercent"])
        generic = copy.deepcopy(self.source)
        source_ids = {logic["logicId"] for _, logic in self.sources}
        for logic in generic["logics"]:
            if logic["logicId"] in source_ids:
                logic.pop("cardSymbols")
        generic_rows, _ = self.project(generic)
        self.assertEqual([{k: v for k, v in row.items() if k != "cardSymbols"} for row in rows], generic_rows)

    def test_selected_projection_rejects_invalid_consumed_card_symbols_without_mutation(self):
        for occurrence, logic in self.sources:
            for invalid in (["NONE"], ["HEART", "SPADE"]):
                candidate = copy.deepcopy(self.source)
                next(row for row in candidate["logics"] if row["logicId"] == logic["logicId"])["cardSymbols"] = invalid
                before = copy.deepcopy(candidate)
                with self.subTest(occurrence=occurrence["occurrenceId"], invalid=invalid), self.assertRaises(projector.CompositionError):
                    self.project(candidate)
                self.assertEqual(before, candidate)

    def test_canonical_rows_emit_symbols_between_parent_and_hit_without_publishing(self):
        for index, (_, logic) in enumerate(self.sources):
            logic["projectileHits"][0]["damagePercent"] = 50 + index
        canonical = [ROOT / projector.SOURCE_PATH, ROOT / projector.ENCOUNTER_PATH,
                     ROOT / projector.PRESENTATION_PATH, ROOT / "Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap"]
        before = {path: path.read_bytes() for path in canonical}
        with tempfile.TemporaryDirectory(dir=ROOT / "out", prefix="DiceContract-") as temporary:
            output = Path(temporary)
            source = output / "snapshot.json"
            source.write_text(json.dumps(self.source), encoding="utf-8")
            result = subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
                str(ROOT / "Tools/KoukuSaydonPipeline/Prepare-KoukuDraftPlay.ps1"), "-RepoRoot", str(ROOT),
                "-SourcePath", str(source), "-OutputDirectory", str(output / "request"), "-PatternId", PATTERN_ID],
                capture_output=True, text=True, encoding="utf-8", errors="replace", timeout=60)
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)
            rows = [line.split("\t") for line in (output / "request/gameplay.rows").read_text().splitlines()]
            selected = [row for row in rows if row[0] in ("PATTERNPURSUITPROJECTILES", "PATTERNPURSUITCARDS", "PATTERNATTACKHIT")]
            self.assertEqual(3 * len(self.sources), len(selected))
            for occurrence, logic in self.sources:
                group = [row for row in selected if row[2] == PATTERN_ID and row[3] == occurrence["occurrenceId"]]
                self.assertEqual(["PATTERNPURSUITPROJECTILES", "PATTERNPURSUITCARDS", "PATTERNATTACKHIT"], [row[0] for row in group])
                self.assertEqual(",".join(logic["cardSymbols"]), group[1][-1])
                self.assertEqual(["MAX_HP_PERCENT", str(logic["projectileHits"][0]["damagePercent"]), "-"], group[2][-3:])
        self.assertEqual(before, {path: path.read_bytes() for path in canonical})


if __name__ == "__main__":
    unittest.main()
