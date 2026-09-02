#!/usr/bin/env python3
"""Focused contract for the project-tuned Valtan counter pulse promotion.

The user-authored V1 document remains the visual source.  Product playback uses
one typed V2 leaf, one reusable group, and three explicit ONCE bindings per
counter window so the count does not depend on the unimplemented EACH_LOOP
runtime policy.
"""

from __future__ import annotations

import json
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
EFFECT_TOOL_V2 = ROOT / "Tools/EffectToolV2"
if str(EFFECT_TOOL_V2) not in sys.path:
    sys.path.insert(0, str(EFFECT_TOOL_V2))

import effect_v2_binding_pipeline as binding_pipeline  # noqa: E402


V1_SOURCE = (
    ROOT
    / "Data/Effects/Authored/effect.valtan.project-tuned.sequence.trash.effect.json"
)
V2_LEAF = (
    ROOT
    / "Data/Effects/V2/Authored/"
    "boss.valtan.project-tuned.sequence.trash.pulse.effectv2.json"
)
V2_GROUP = (
    ROOT
    / "Data/Effects/V2/Groups/"
    "boss.valtan.project-tuned.sequence.trash.pulse-group.effectv2group.json"
)
BINDINGS = ROOT / "Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json"
PRESENTATION = ROOT / "Data/Valtan/Valtan.presentation.json"
GAMEPLAY = ROOT / "Data/Valtan/Valtan.gameplay.json"
EFFECT_CATALOG = ROOT / "Data/Effects/EffectCatalog.json"
DRAFT_OWNERS = ROOT / "Data/Effects/ValtanPatternAuthoringEffects.json"
PATTERN_CUES = (
    ROOT / "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
)

LEAF_ID = "boss.valtan.project-tuned.sequence.trash.pulse"
GROUP_ID = "boss.valtan.project-tuned.sequence.trash.pulse-group"
LIFETIME_SECONDS = 0.6
LIFETIME_MS = 600

TRASH_WINDOWS = {
    (
        "VALTAN_TRASH",
        "STEP_07",
        "valtan.sequence.center-trash-rush-if.step-07",
    ): "valtan.sequence.center-trash-rush-if.step-07.clip-01",
    (
        "VALTAN_TRASH",
        "RETRY_WINDUP_02",
        "valtan.sequence.center-trash-rush-if.retry-windup-02",
    ): "valtan.sequence.center-trash-rush-if.retry-windup-02.clip-01",
    (
        "VALTAN_TRASH",
        "RETRY_WINDUP_03",
        "valtan.sequence.center-trash-rush-if.retry-windup-03",
    ): "valtan.sequence.center-trash-rush-if.retry-windup-03.clip-01",
}

COUNTER_WINDOWS = {
    (
        "VALTAN_COUNTER",
        "STEP_02",
        "valtan.sequence.counter.step-02",
    ): (
        "valtan.sequence.counter.step-02.clip-01",
        "valtan.sequence.counter.step-02.clip-02",
    ),
    (
        "VALTAN_TRIPLE_COUNTER",
        "COUNTER_1",
        "valtan.reactive.triple-counter.first",
    ): (
        "valtan.reactive.triple-counter.first.clip.01",
        "valtan.reactive.triple-counter.first.clip.02",
    ),
    (
        "VALTAN_TRIPLE_COUNTER",
        "COUNTER_2",
        "valtan.reactive.triple-counter.second",
    ): (
        "valtan.reactive.triple-counter.second.clip.01",
        "valtan.reactive.triple-counter.second.clip.02",
    ),
    (
        "VALTAN_TRIPLE_COUNTER",
        "COUNTER_3",
        "valtan.reactive.triple-counter.third",
    ): (
        "valtan.reactive.triple-counter.third.clip.01",
        "valtan.reactive.triple-counter.third.clip.02",
    ),
}


def read_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def stage_index(document: dict) -> dict[tuple[str, str, str], dict]:
    return {
        (pattern["patternId"], stage["stageId"], stage["actionId"]): stage
        for pattern in document["patterns"]
        for stage in pattern["stages"]
    }


class ValtanProjectTunedCounterPulseEffectContract(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.v1 = read_json(V1_SOURCE)
        cls.leaf = read_json(V2_LEAF)
        cls.group = read_json(V2_GROUP)
        cls.bindings = read_json(BINDINGS)["bindings"]
        cls.presentation = stage_index(read_json(PRESENTATION))
        cls.gameplay = stage_index(read_json(GAMEPLAY))
        cls.effect_catalog = read_json(EFFECT_CATALOG)
        cls.draft_owners = read_json(DRAFT_OWNERS)
        cls.pattern_cues = read_json(PATTERN_CUES)

    def test_v1_particle_visual_is_promoted_without_inheriting_its_delay(self) -> None:
        self.assertEqual(
            "effect.valtan.project-tuned.sequence.trash",
            self.v1["effectAssetId"],
        )
        self.assertEqual(1, len(self.v1["elements"]))
        source = self.v1["elements"][0]
        self.assertEqual("particle", source["kind"])
        source_resources = {
            row["slotId"]: row["assetId"] for row in source["resources"]
        }
        self.assertEqual(
            "Effect/DimensionMaster/Meshes/fm_b_ring_001.wmodel",
            source_resources["meshModel"],
        )
        texture = "Effect/DimensionMaster/Textures/FX_TEX_00/fx_a_noise_003.dds"
        for slot in ("base", "mask", "emissive"):
            self.assertEqual(texture, source_resources[slot])
        for seconds in source["detail"]["particle"]["lifeTimeSeconds"]:
            self.assertAlmostEqual(LIFETIME_SECONDS, seconds, places=6)
        timing = source["detail"]["timing"]
        self.assertEqual(0, timing["startDelaySeconds"])
        self.assertEqual(LIFETIME_SECONDS, timing["lifeTimeSeconds"])

        self.assertEqual("lostark.effect-v2", self.leaf["schema"])
        self.assertEqual(LEAF_ID, self.leaf["effectId"])
        self.assertEqual("Particle", self.leaf["effectType"])
        self.assertEqual(source_resources["meshModel"], self.leaf["slots"]["mesh"])
        for slot in ("base", "mask", "emissive"):
            self.assertEqual(source_resources[slot], self.leaf["slots"][slot])
        params = self.leaf["params"]
        self.assertNotIn("startDelaySeconds", params)
        self.assertNotIn("startDelayMs", params)
        self.assertEqual(LIFETIME_SECONDS, params["lifetime"])
        self.assertFalse(params["loop"])
        self.assertEqual(
            [LIFETIME_SECONDS, LIFETIME_SECONDS],
            params["particle"]["lifetime"],
        )
        self.assertEqual(1, params["particle"]["maxParticles"])
        self.assertEqual(1, params["particle"]["burstCount"])
        self.assertEqual(0.0, params["particle"]["spawnRate"])

    def test_reusable_group_has_one_immediate_six_hundred_ms_leaf(self) -> None:
        self.assertEqual("lostark.effect-v2-group", self.group["schema"])
        self.assertEqual(2, self.group["formatVersion"])
        self.assertEqual(GROUP_ID, self.group["groupId"])
        self.assertEqual(LIFETIME_MS, self.group["durationMs"])
        self.assertEqual(1, len(self.group["children"]))
        child = self.group["children"][0]
        self.assertEqual({"kind": "LEAF", "id": LEAF_ID}, child["resource"])
        self.assertEqual(0, child["startMs"])
        self.assertEqual(LIFETIME_MS, child["durationMs"])
        self.assertEqual("Deactivate", child["stop"])

        authored, groups = binding_pipeline._load_resource_documents(ROOT)
        leaves, resolved_span_ms = binding_pipeline._resolve_group(
            GROUP_ID,
            authored,
            groups,
            require_v2=True,
        )
        self.assertEqual(LIFETIME_MS, resolved_span_ms)
        self.assertEqual([(LEAF_ID, 0)], [(leaf[0], leaf[1]) for leaf in leaves])

    def test_v1_source_is_draft_only_and_cannot_spawn_a_second_runtime_copy(self) -> None:
        source_id = "effect.valtan.project-tuned.sequence.trash"
        self.assertNotIn(
            source_id,
            {row["effectAssetId"] for row in self.effect_catalog["effects"]},
        )
        self.assertEqual(
            [
                {
                    "patternId": "VALTAN_TRASH",
                    "effectAssetId": source_id,
                    "authoringPath": (
                        "Effects/Authored/"
                        "effect.valtan.project-tuned.sequence.trash.effect.json"
                    ),
                    "state": "DRAFT_ATTACHED",
                }
            ],
            [
                row
                for row in self.draft_owners["bindings"]
                if row["effectAssetId"] == source_id
            ],
        )
        stale_cue_id = "cue.valtan.requested.20260827.trash.composite"
        authored_cues = self.pattern_cues["cues"]
        projected_cues = [
            cue
            for stage in self.presentation.values()
            for cue in stage["effectCues"]
        ]
        for cues in (authored_cues, projected_cues):
            self.assertFalse(
                any(
                    cue.get("bindingId") == stale_cue_id
                    or cue.get("cueId") == stale_cue_id
                    or cue.get("effectAssetId") == source_id
                    for cue in cues
                )
            )

    def test_three_explicit_pulses_fit_fully_inside_every_target_sequence_window(self) -> None:
        group_rows = [
            row
            for row in self.bindings
            if row["resource"] == {"kind": "GROUP", "id": GROUP_ID}
        ]
        self.assertEqual(21, len(group_rows))
        self.assertEqual(
            [
                f"binding.valtan.project-tuned.counter-pulse.{index:03d}"
                for index in range(1, 22)
            ],
            [row["bindingId"] for row in group_rows],
        )

        expected_scopes = set(TRASH_WINDOWS) | set(COUNTER_WINDOWS)
        actual_scopes = {
            (
                row["scope"]["patternId"],
                row["scope"]["stageId"],
                row["scope"]["actionId"],
            )
            for row in group_rows
        }
        self.assertEqual(expected_scopes, actual_scopes)

        for scope in sorted(expected_scopes):
            rows = [
                row
                for row in group_rows
                if (
                    row["scope"]["patternId"],
                    row["scope"]["stageId"],
                    row["scope"]["actionId"],
                )
                == scope
            ]
            self.assertEqual(3, len(rows), scope)
            for row in rows:
                self.assertEqual("CLIP_OCCURRENCE", row["clock"]["basis"])
                self.assertEqual("ONCE", row["clock"]["repeatPolicy"])
                self.assertEqual("NATURAL", row["stopPolicy"])
                self.assertEqual("b_effectroot", row["anchor"]["slotId"])
                self.assertEqual("FOLLOW_SLOT", row["anchor"]["followPolicy"])
                self.assertEqual("TARGET_YAW", row["anchor"]["rotationBasis"])

            presentation = self.presentation[scope]
            gameplay = self.gameplay[scope]
            occurrences = presentation["animation"]["occurrences"]
            self.assertEqual("EXACT", presentation["animation"]["endPolicy"])
            self.assertEqual(
                gameplay["durationMs"],
                sum(occurrence["playMs"] for occurrence in occurrences),
            )
            by_id = {
                occurrence["clipOccurrenceId"]: occurrence
                for occurrence in occurrences
            }
            occurrence_global_start = {}
            elapsed_ms = 0
            for occurrence in occurrences:
                occurrence_global_start[occurrence["clipOccurrenceId"]] = elapsed_ms
                elapsed_ms += occurrence["playMs"]

            global_starts = []
            for row in rows:
                clock = row["clock"]
                occurrence = by_id[clock["clipOccurrenceId"]]
                self.assertFalse(occurrence["repeatUntilStageEnd"])
                self.assertLess(
                    clock["startMs"], occurrence["playMs"], row["bindingId"]
                )
                global_start = (
                    occurrence_global_start[clock["clipOccurrenceId"]]
                    + clock["startMs"]
                )
                self.assertLessEqual(
                    global_start + LIFETIME_MS,
                    gameplay["durationMs"],
                    row["bindingId"],
                )
                global_starts.append(global_start)

            if scope in TRASH_WINDOWS:
                self.assertEqual(1000, gameplay["durationMs"])
                for row in rows:
                    occurrence = by_id[row["clock"]["clipOccurrenceId"]]
                    self.assertLessEqual(
                        row["clock"]["startMs"] + LIFETIME_MS,
                        occurrence["playMs"],
                        row["bindingId"],
                    )
                self.assertEqual(
                    [(TRASH_WINDOWS[scope], "mesh_att_battle_13_03", 1000)],
                    [
                        (
                            occurrence["clipOccurrenceId"],
                            occurrence["clip"],
                            occurrence["playMs"],
                        )
                        for occurrence in occurrences
                    ],
                )
                self.assertEqual([0, 200, 400], sorted(global_starts))
            else:
                self.assertEqual(1800, gameplay["durationMs"])
                self.assertEqual(
                    [
                        (COUNTER_WINDOWS[scope][0], "mesh_att_battle_14_02", 1000),
                        (COUNTER_WINDOWS[scope][1], "mesh_att_battle_14_02", 800),
                    ],
                    [
                        (
                            occurrence["clipOccurrenceId"],
                            occurrence["clip"],
                            occurrence["playMs"],
                        )
                        for occurrence in occurrences
                    ],
                )
                self.assertEqual([0, 600, 1200], sorted(global_starts))


if __name__ == "__main__":
    unittest.main()
