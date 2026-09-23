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

class DiceCardContractTests(unittest.TestCase):
    def setUp(self):
        self.source = json.loads((ROOT / projector.SOURCE_PATH).read_text(encoding="utf-8-sig"))
        self.logic = next(row for row in self.source["logics"] if row["logicId"] == "kakulsaydon.g1.logic.73")
        self.logic["cardSymbols"] = ["HEART", "SPADE", "CLUB", "DIAMOND"]
        self.logic["projectileHits"][0]["damagePercent"] = 50

    def test_symbol_contract_rejects_bad_values_and_preserves_optional_generic_pursuit(self):
        for bad in ([], ["HEART"], ["HEART", "SPADE", "CLUB", "NONE"],
                    ["HEART", "SPADE", "CLUB", "red"], ["HEART", "SPADE", "CLUB", []]):
            with self.subTest(bad=bad), self.assertRaises(projector.CompositionError):
                projector._pursuit_fields({**self.logic, "cardSymbols": bad}, "Dice")
        self.assertEqual(self.logic["cardSymbols"], projector._pursuit_fields(self.logic, "Dice")["cardSymbols"])
        generic = copy.deepcopy(self.logic); generic.pop("cardSymbols")
        self.assertNotIn("cardSymbols", projector._pursuit_fields(generic, "Generic pursuit"))

    def test_selected_projection_keeps_visual_order_color_and_source_immutable(self):
        before = copy.deepcopy(self.source)
        metadata, artifacts = draft.prepare(self.source, ROOT, pattern_id="KAKULSAYDON_G1_PATTERN_78")
        self.assertEqual(before, self.source)
        encounter = json.loads(artifacts[Path("encounter.json")])
        pattern = next(p for p in encounter["patterns"] if p["patternId"] == "KAKULSAYDON_G1_PATTERN_78")
        row = pattern["pursuitProjectiles"][0]
        self.assertEqual(self.logic["cardSymbols"], row["cardSymbols"])
        self.assertEqual(50, row["projectileHits"][0]["damagePercent"])
        generic = copy.deepcopy(self.source)
        next(x for x in generic["logics"] if x["logicId"] == self.logic["logicId"]).pop("cardSymbols")
        _, legacy = draft.prepare(generic, ROOT, pattern_id="KAKULSAYDON_G1_PATTERN_78")
        self.assertEqual(artifacts[Path("presentation.json")], legacy[Path("presentation.json")])

    def test_canonical_rows_emit_symbols_between_parent_and_hit_without_publishing(self):
        canonical = [ROOT / projector.SOURCE_PATH, ROOT / projector.ENCOUNTER_PATH,
                     ROOT / projector.PRESENTATION_PATH, ROOT / "Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap"]
        before = {p: p.read_bytes() for p in canonical}
        with tempfile.TemporaryDirectory(dir=ROOT / "out", prefix="DiceContract-") as temporary:
            out = Path(temporary); source = out / "snapshot.json"
            source.write_text(json.dumps(self.source), encoding="utf-8")
            result = subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
                str(ROOT / "Tools/KoukuSaydonPipeline/Prepare-KoukuDraftPlay.ps1"), "-RepoRoot", str(ROOT),
                "-SourcePath", str(source), "-OutputDirectory", str(out / "request"),
                "-PatternId", "KAKULSAYDON_G1_PATTERN_78"], capture_output=True, text=True, encoding="utf-8", errors="replace", timeout=60)
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)
            rows = [line.split("\t") for line in (out / "request/gameplay.rows").read_text().splitlines()]
            selected = [r for r in rows if r[0] in ("PATTERNPURSUITPROJECTILES", "PATTERNPURSUITCARDS", "PATTERNATTACKHIT")]
            self.assertEqual(["PATTERNPURSUITPROJECTILES", "PATTERNPURSUITCARDS", "PATTERNATTACKHIT"], [r[0] for r in selected])
            self.assertEqual("HEART,SPADE,CLUB,DIAMOND", selected[1][-1])
            self.assertEqual(["MAX_HP_PERCENT", "50", "-"], selected[2][-3:])
        self.assertEqual(before, {p: p.read_bytes() for p in canonical})

if __name__ == "__main__":
    unittest.main()
