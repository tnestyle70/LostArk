from __future__ import annotations

import copy
import json
import unittest
from pathlib import Path

import valtan_tuning_pipeline as pipeline


ROOT = Path(__file__).resolve().parents[2]


class LandingEffectAnchorTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.docs = pipeline.load_pipeline_documents(ROOT)
        cls.master = pipeline.join_v2_authoring(
            cls.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            cls.docs[pipeline.PRESENTATION_AUTHORING_REL],
            cls.docs[pipeline.WORLD_SET_REL],
            cls.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        cls.catalog = json.loads((ROOT / "Data/Effects/EffectCatalog.json").read_text(encoding="utf-8"))

    def candidate(self):
        master = copy.deepcopy(self.master)
        pattern = next(p for p in master["patterns"] if p["patternId"] == "VALTAN_HIGH_JUMP")
        selected = []
        for stage in pattern["stages"]:
            if stage["stageId"] not in ("AIRBORNE", "LAND"):
                continue
            for cue in stage["effectCues"]:
                if cue["cueId"] in (
                    "cue.valtan.composition.valtan_high_jump.airborne.01",
                    "cue.valtan.composition.valtan_high_jump.land.01",
                ):
                    cue["anchorSlotId"] = "pattern.landing.snapshot"
                    cue["followPolicy"] = "snapshot"
                    selected.append((stage, cue))
        self.assertEqual(2, len(selected))
        return master, pattern, selected

    def validate(self, master):
        pipeline.validate_v2_master(master, self.docs[pipeline.WORLD_SET_REL], self.docs[pipeline.COMBAT_AUTHORING_REL])

    def test_projection_preserves_landing_anchor_and_effect_timing(self):
        master, _, selected = self.candidate()
        self.validate(master)
        projected = pipeline.project_v2_products(ROOT, self.docs, master)
        cues = {c["bindingId"]: c for c in json.loads(projected[pipeline.CUES_REL])["cues"]}
        for _, source in selected:
            cue = cues[source["cueId"]]
            self.assertEqual("pattern.landing.snapshot", cue["anchorSlotId"])
            self.assertEqual("snapshot", cue["followPolicy"])
            for key in ("effectAssetId", "sourceStartMs", "sourceEndMs", "localTransform", "scalePolicy"):
                self.assertEqual(source[key], cue[key])
            self.assertEqual(source.get("playbackOffsetMs"), cue.get("playbackOffsetMs"))

    def test_draft_accepts_explicit_landing_for_both_cues(self):
        _, pattern, selected = self.candidate()
        for stage, cue in selected:
            pipeline._validate_draft_effect_cue_payload(ROOT, self.catalog, pattern, stage, cue, 0)

    def test_both_admission_paths_reject_follow_unknown_or_non_leap(self):
        mutations = (
            lambda p, c: c.update(followPolicy="follow"),
            lambda p, c: c.update(anchorSlotId="pattern.landing.unknown"),
            lambda p, c: p.update(serverMotion=None),
        )
        for mutate in mutations:
            with self.subTest(mutation=mutate):
                master, pattern, selected = self.candidate()
                stage, cue = selected[0]
                mutate(pattern, cue)
                with self.assertRaises(pipeline.PipelineError):
                    self.validate(master)
                with self.assertRaises(pipeline.PipelineError):
                    pipeline._validate_draft_effect_cue_payload(ROOT, self.catalog, pattern, stage, cue, 0)


if __name__ == "__main__":
    unittest.main()
