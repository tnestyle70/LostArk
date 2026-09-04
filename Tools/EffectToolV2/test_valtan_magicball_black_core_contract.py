import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
AUTHORED = ROOT / "Data" / "Effects" / "V2" / "Authored"
GROUP_PATH = (
    ROOT
    / "Data"
    / "Effects"
    / "V2"
    / "Groups"
    / "boss.valtan.magicball.effectv2group.json"
)
AURA_GROUP_PATH = (
    ROOT
    / "Data"
    / "Effects"
    / "V2"
    / "Groups"
    / "boss.valtan.magicball.aura.effectv2group.json"
)
BINDINGS_PATH = (
    ROOT
    / "Data"
    / "Effects"
    / "V2"
    / "Bindings"
    / "BOSS_VALTAN.effectv2bindings.json"
)


def load_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def load_effect(effect_id: str) -> dict:
    return load_json(AUTHORED / f"{effect_id}.effectv2.json")


class ValtanMagicballBlackCoreContractTests(unittest.TestCase):
    def test_stage_binding_and_group_cover_the_full_channel(self) -> None:
        bindings = load_json(BINDINGS_PATH)["bindings"]
        rows = [
            row
            for row in bindings
            if row["resource"]
            == {"kind": "GROUP", "id": "boss.valtan.magicball"}
        ]
        self.assertEqual(1, len(rows))
        self.assertEqual(
            {
                "patternId": "VALTAN_STAGGER_SLOT",
                "stageId": "CHANNEL",
                "actionId": "valtan.authoring.stagger-slot.channel",
            },
            rows[0]["scope"],
        )
        self.assertEqual("STAGE", rows[0]["clock"]["basis"])
        self.assertEqual(0, rows[0]["clock"]["startMs"])
        self.assertEqual("ONCE", rows[0]["clock"]["repeatPolicy"])

        group = load_json(GROUP_PATH)
        self.assertEqual(12000, group["durationMs"])
        lifetimes = {
            child["resource"]["id"]: load_effect(child["resource"]["id"])["params"][
                "lifetime"
            ]
            for child in group["children"]
        }
        self.assertEqual(12.0, lifetimes["boss.valtan.egg.black_3"])
        self.assertEqual(12.0, lifetimes["boss.valtan.egg.cyan_1"])
        self.assertEqual(12.0, lifetimes["boss.valtan.egg.black_1"])
        self.assertEqual(12.0, lifetimes["boss.valtan.egg.black_2"])

    def test_black_multiply_layer_is_composited_after_the_cyan_shell(self) -> None:
        group = load_json(GROUP_PATH)
        child_ids = [child["resource"]["id"] for child in group["children"]]
        self.assertEqual(
            [
                "boss.valtan.egg.black_1",
                "boss.valtan.egg.black_2",
                "boss.valtan.egg.cyan_1",
                "boss.valtan.egg.black_3",
            ],
            child_ids,
        )

        cyan = load_effect("boss.valtan.egg.cyan_1")["params"]
        black = load_effect("boss.valtan.egg.black_3")["params"]
        self.assertEqual("Additive", cyan["blend"])
        self.assertEqual("Multiply", black["blend"])
        self.assertTrue(black["depthTest"])
        self.assertEqual([0.0, 0.0, 0.0, 1.0], black["colorMul"])
        self.assertEqual(cyan["position"]["start"], black["position"]["start"])
        self.assertEqual([0.0, 1.5, 3.0], black["position"]["start"])
        self.assertTrue(
            all(
                black_axis > cyan_axis
                for black_axis, cyan_axis in zip(
                    black["scale"]["start"], cyan["scale"]["start"]
                )
            )
        )

    def test_channel_core_and_aura_share_the_stage_clock_and_stop_with_stage(self) -> None:
        bindings = load_json(BINDINGS_PATH)["bindings"]
        resources = {
            "boss.valtan.magicball",
            "boss.valtan.magicball.aura",
        }
        rows = [
            row
            for row in bindings
            if row["resource"]["kind"] == "GROUP"
            and row["resource"]["id"] in resources
            and row["scope"]["actionId"] == "valtan.authoring.stagger-slot.channel"
        ]
        self.assertEqual(resources, {row["resource"]["id"] for row in rows})
        self.assertEqual(2, len(rows))
        for row in rows:
            self.assertEqual(
                {
                    "patternId": "VALTAN_STAGGER_SLOT",
                    "stageId": "CHANNEL",
                    "actionId": "valtan.authoring.stagger-slot.channel",
                },
                row["scope"],
            )
            self.assertEqual(
                {
                    "basis": "STAGE",
                    "clipOccurrenceId": None,
                    "startMs": 0,
                    "repeatPolicy": "ONCE",
                },
                row["clock"],
            )
            self.assertEqual("STAGE_END", row["stopPolicy"])

        aura_group = load_json(AURA_GROUP_PATH)
        self.assertEqual(0, aura_group["durationMs"])
        self.assertEqual(2, len(aura_group["children"]))
        for child in aura_group["children"]:
            effect = load_effect(child["resource"]["id"])
            self.assertEqual("Mesh", effect["effectType"])
            self.assertEqual(12.0, effect["params"]["lifetime"])
            self.assertFalse(effect["params"]["loop"])

        runtime = (ROOT / "Client" / "Private" / "EffectV2_Runtime.cpp").read_text(
            encoding="utf-8"
        )
        document = (ROOT / "Client" / "Private" / "EffectV2_Document.cpp").read_text(
            encoding="utf-8"
        )
        self.assertIn("Prune_Spawned(State, false, true);", runtime)
        self.assertIn(
            "EFFECT_V2_STOP_POLICY::STAGE_END == Binding.eStopPolicy", document
        )

    def test_runtime_preserves_equal_depth_group_submission_order(self) -> None:
        runtime = (ROOT / "Client" / "Private" / "EffectV2_Runtime.cpp").read_text(
            encoding="utf-8"
        )
        layer = (ROOT / "Engine" / "Private" / "Layer.cpp").read_text(
            encoding="utf-8"
        )
        renderer = (ROOT / "Engine" / "Private" / "Renderer.cpp").read_text(
            encoding="utf-8"
        )
        self.assertIn(
            "for (size_t iChild = 0u; iChild < Group.Children.size(); ++iChild)",
            runtime,
        )
        self.assertIn("m_GameObjects.push_back(pGameObject);", layer)
        self.assertIn("std::stable_sort(", renderer)


if __name__ == "__main__":
    unittest.main()
