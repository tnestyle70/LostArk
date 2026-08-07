from __future__ import annotations

import importlib.util
import json
from pathlib import Path
import tempfile
import unittest


SCRIPT_PATH = Path(__file__).with_name("seed_dimensionmaster_a_candidate.py")
SPEC = importlib.util.spec_from_file_location("seed_dimensionmaster_a_candidate", SCRIPT_PATH)
assert SPEC is not None and SPEC.loader is not None
seed = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(seed)


def resource(layer: dict, slot_id: str) -> str:
    for binding in layer["resources"]:
        if binding["slotId"] == slot_id:
            return binding["assetId"]
    return ""


class DimensionMasterACandidateTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        with seed.DEFAULT_SOURCE.open("r", encoding="utf-8") as stream:
            cls.source = json.load(stream)

    def test_builds_four_hit_groups_with_four_manual_mesh_layers_each(self) -> None:
        candidate = seed.build_candidate(self.source)
        self.assertEqual(seed.CANDIDATE_EFFECT_ID, candidate["effectAssetId"])
        self.assertEqual(16, len(candidate["elements"]))
        self.assertEqual([], candidate["modelCues"])
        self.assertEqual(
            [
                f"manual.a.hit{hit_index:02d}.{layer_name}"
                for hit_index in range(1, 5)
                for layer_name in (
                    "body",
                    "rim",
                    "highlight",
                    "afterimage",
                )
            ],
            [layer["id"] for layer in candidate["elements"]],
        )
        expected_delays = {
            "manual.a.hit01": 0.25,
            "manual.a.hit02": 0.60,
            "manual.a.hit03": 0.90,
            "manual.a.hit04": 1.30,
        }
        for layer in candidate["elements"]:
            self.assertEqual("mesh", layer["kind"])
            self.assertEqual("effect.standard", layer["material"]["templateId"])
            self.assertFalse(layer["detail"]["mesh"]["useModelMaterial"])
            self.assertFalse(layer["sourceRecipe"]["enabled"])
            self.assertFalse(layer["sourcePresentation"]["enabled"])
            self.assertFalse(layer["actionCueAttachment"]["enabled"])
            self.assertEqual(
                expected_delays[layer["groupId"]],
                layer["detail"]["timing"]["startDelaySeconds"],
            )

    def test_preserves_evidence_based_body_and_rim_resources(self) -> None:
        candidate = seed.build_candidate(self.source)
        layers = {layer["id"]: layer for layer in candidate["elements"]}
        body = layers["manual.a.hit01.body"]
        rim = layers["manual.a.hit01.rim"]
        self.assertTrue(resource(body, "meshModel").endswith("fm_h_swing_02.wmodel"))
        self.assertTrue(resource(body, "base").endswith("fx_j_mirnoise_02.dds"))
        self.assertTrue(resource(body, "noise").endswith("fx_d_noise_014.dds"))
        self.assertTrue(resource(body, "mask").endswith("fx_j_auraline_19_ycl.dds"))
        self.assertTrue(resource(rim, "base").endswith("fx_l_environment_001.dds"))
        self.assertTrue(resource(rim, "mask").endswith("fx_j_line_01_xcl.dds"))
        self.assertEqual("", resource(rim, "noise"))

    def test_uses_particle_life_and_size_evidence_as_manual_seed(self) -> None:
        candidate = seed.build_candidate(self.source)
        layers = {layer["id"]: layer for layer in candidate["elements"]}
        self.assertEqual(0.5, layers["manual.a.hit01.body"]["detail"]["timing"]["lifeTimeSeconds"])
        self.assertEqual(
            [0.0341, 0.0341, 0.0341],
            layers["manual.a.hit01.body"]["detail"]["transform"]["scale"],
        )
        self.assertEqual(0.3, layers["manual.a.hit01.rim"]["detail"]["timing"]["lifeTimeSeconds"])
        self.assertEqual(0.2, layers["manual.a.hit01.highlight"]["detail"]["timing"]["lifeTimeSeconds"])
        self.assertEqual(0.7, layers["manual.a.hit01.afterimage"]["detail"]["timing"]["lifeTimeSeconds"])
        self.assertFalse(layers["manual.a.hit01.afterimage"]["visible"])

    def test_tilts_the_xz_swing_mesh_about_player_local_x(self) -> None:
        candidate = seed.build_candidate(self.source)
        layers = {layer["id"]: layer for layer in candidate["elements"]}
        expected_positions = {
            "hit01": [0.5, 0.15, -0.9],
            "hit02": [0.5, 0.15, 0.8],
            "hit03": [0.5, 0.3, -0.9],
            "hit04": [0.5, 0.6, -0.8],
        }
        for hit_id, expected_position in expected_positions.items():
            for layer_name in ("body", "rim", "highlight"):
                layer_id = f"manual.a.{hit_id}.{layer_name}"
                transform = layers[layer_id]["detail"]["transform"]
                linear = layers[layer_id]["detail"]["linearLerp"]
                self.assertEqual(expected_position, transform["position"])
                self.assertEqual(
                    [-18.0, 0.0, 0.0], transform["rotationDegrees"]
                )
                self.assertEqual(
                    [-280.0, 0.0, 0.0],
                    transform["revolutionDegreesPerSecond"],
                )
                self.assertEqual(
                    [-70.0, 0.0, 0.0],
                    linear["endRevolutionDegreesPerSecond"],
                )

    def test_checked_in_candidate_matches_the_deterministic_seed(self) -> None:
        with seed.DEFAULT_OUTPUT.open("r", encoding="utf-8") as stream:
            checked_in = json.load(stream)
        self.assertEqual(seed.build_candidate(self.source), checked_in)

    def test_rejects_wrong_source_identity_and_missing_required_element(self) -> None:
        wrong = json.loads(json.dumps(self.source))
        wrong["effectAssetId"] = "effect.dimensionmaster.skill.wrong"
        with self.assertRaisesRegex(ValueError, "not canonical"):
            seed.build_candidate(wrong)

        missing = json.loads(json.dumps(self.source))
        missing["elements"] = [
            element
            for element in missing["elements"]
            if element["id"] != seed.BODY_SOURCE_ID
        ]
        with self.assertRaisesRegex(ValueError, "source element is missing"):
            seed.build_candidate(missing)

    def test_atomic_writer_refuses_existing_candidate(self) -> None:
        candidate = seed.build_candidate(self.source)
        authored_root = seed.REPOSITORY_ROOT / "Data/Effects/Authored"
        with tempfile.TemporaryDirectory(dir=authored_root) as temporary:
            output = Path(temporary) / "candidate.effect.json"
            with self.assertRaisesRegex(ValueError, "directly under"):
                seed.write_candidate(output, candidate)

        output = authored_root / "effect.dimensionmaster.skill.2050210.test-seed.effect.json"
        try:
            seed.write_candidate(output, candidate)
            with self.assertRaisesRegex(FileExistsError, "already exists"):
                seed.write_candidate(output, candidate)
        finally:
            output.unlink(missing_ok=True)


if __name__ == "__main__":
    unittest.main()
