from __future__ import annotations

import copy
import importlib.util
import json
import pathlib
import tempfile
import unittest


SCRIPT_PATH = pathlib.Path(__file__).with_name(
    "apply_artist_31420_grass_tip_fade.py"
)
SPEC = importlib.util.spec_from_file_location("artist_31420_grass_tip", SCRIPT_PATH)
assert SPEC is not None and SPEC.loader is not None
module = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(module)


class Artist31420GrassTipFadeTests(unittest.TestCase):
    def _temporary_document(self, document: dict) -> pathlib.Path:
        directory = tempfile.TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        path = pathlib.Path(directory.name) / module.DOCUMENT_PATH.name
        path.write_text(
            json.dumps(document, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
            newline="",
        )
        return path

    def test_repository_composition_is_admitted(self) -> None:
        module.run(write=False)

    def test_append_is_atomic_and_idempotent(self) -> None:
        document = json.loads(
            module.DOCUMENT_PATH.read_text(encoding="utf-8-sig")
        )
        document["elements"] = document["elements"][: module.BASE_ROW_COUNT]
        path = self._temporary_document(document)
        self.assertTrue(module.run(write=True, document_path=path))
        first = path.read_bytes()
        self.assertFalse(module.run(write=True, document_path=path))
        self.assertEqual(path.read_bytes(), first)
        result = json.loads(path.read_text(encoding="utf-8-sig"))
        self.assertEqual(
            result["elements"][module.BASE_ROW_COUNT :],
            module.build_project_rows(),
        )

    def test_source_row_and_project_row_drift_fail_closed(self) -> None:
        document = json.loads(
            module.DOCUMENT_PATH.read_text(encoding="utf-8-sig")
        )
        source_drift = copy.deepcopy(document)
        source_drift["elements"][0]["visible"] = False
        with self.assertRaises(module.ArtistGrassTipError):
            module.validate_document(source_drift)
        project_drift = copy.deepcopy(document)
        project_drift["elements"][-1]["detail"]["color"][
            "emissiveIntensity"
        ] = 0
        with self.assertRaises(module.ArtistGrassTipError):
            module.validate_document(project_drift)

    def test_grass_textures_are_explicit_coverage_and_dissolve_lanes(self) -> None:
        body, tip = module.build_project_rows()
        expected_roles = [
            {
                "mask": module.GRASS_04_ASSET_ID,
                "dissolve": module.GRASS_03_ASSET_ID,
            },
            {
                "mask": module.GRASS_03_ASSET_ID,
                "dissolve": module.GRASS_04_ASSET_ID,
            },
        ]
        for row, roles in zip((body, tip), expected_roles):
            resources = {
                item["slotId"]: item["assetId"] for item in row["resources"]
            }
            self.assertEqual(resources["base"], module.LINE_ASSET_ID)
            self.assertEqual(resources["emissive"], module.EMISSIVE_ASSET_ID)
            self.assertEqual(resources["mask"], roles["mask"])
            self.assertEqual(resources["dissolve"], roles["dissolve"])
            self.assertNotIn(resources["base"], roles.values())
            self.assertEqual(row["material"]["templateId"], "effect.standard")
            self.assertEqual(
                row["material"]["renderProfile"],
                "alpha_two_sided_depth_read",
            )

    def test_body_and_tip_share_one_typed_rect_family(self) -> None:
        body, tip = module.build_project_rows()
        for row in (body, tip):
            execution = row["material"]["execution"]
            self.assertTrue(execution["enabled"])
            self.assertEqual(execution["backend"], "runtimeMaterialV2")
            self.assertEqual(execution["opcode"], module.RUNTIME_MATERIAL_OPCODE)
            self.assertEqual(execution["passIndex"], 1)
            self.assertEqual(execution["textureMask"], 15)
            self.assertEqual(
                [lane["role"] for lane in execution["textureLanes"]],
                [
                    "base_radiance",
                    "coverage",
                    "emissive_radiance",
                    "dissolve",
                ],
            )
            self.assertEqual(
                [lane["sourceChannel"] for lane in execution["textureLanes"]],
                ["RGBA", "R", "RGB", "R"],
            )
            self.assertEqual(
                [lane["colorSpace"] for lane in execution["textureLanes"]],
                ["linear"] * 4,
            )
            self.assertEqual(execution["scalarCount"], 0)
            self.assertEqual(execution["vectorCount"], 0)

        body_lanes = body["material"]["execution"]["textureLanes"]
        tip_lanes = tip["material"]["execution"]["textureLanes"]
        self.assertEqual(body_lanes[0], tip_lanes[0])
        self.assertEqual(body_lanes[2], tip_lanes[2])
        self.assertEqual(body_lanes[1]["assetId"], module.GRASS_04_ASSET_ID)
        self.assertEqual(body_lanes[3]["assetId"], module.GRASS_03_ASSET_ID)
        self.assertEqual(tip_lanes[1]["assetId"], module.GRASS_03_ASSET_ID)
        self.assertEqual(tip_lanes[3]["assetId"], module.GRASS_04_ASSET_ID)

    def test_tip_is_hdr_emissive_not_a_point_light(self) -> None:
        body, tip = module.build_project_rows()
        self.assertGreater(tip["detail"]["color"]["emissiveIntensity"], 1)
        self.assertTrue(tip["detail"]["linearLerp"]["emissiveIntensity"])
        self.assertEqual(
            tip["detail"]["linearLerp"]["endEmissiveIntensity"], 0
        )
        for row in (body, tip):
            self.assertEqual(row["kind"], "sprite")
            self.assertFalse(row["detail"]["light"]["enabled"])
            self.assertTrue(row["detail"]["linearLerp"]["colorMultiply"])
            self.assertEqual(
                row["detail"]["linearLerp"]["endColorMultiply"][3], 0
            )

    def test_grass_and_tip_share_one_absolute_end_time(self) -> None:
        body, tip = module.build_project_rows()
        body_timing = body["detail"]["timing"]
        tip_timing = tip["detail"]["timing"]
        self.assertAlmostEqual(
            body_timing["startDelaySeconds"] + body_timing["lifeTimeSeconds"],
            module.COHORT_END_SECONDS,
        )
        self.assertAlmostEqual(
            tip_timing["startDelaySeconds"] + tip_timing["lifeTimeSeconds"],
            module.COHORT_END_SECONDS,
        )
        self.assertLess(body_timing["dissolveStartNormalized"], 1)
        self.assertLess(tip_timing["dissolveStartNormalized"], 1)

    def test_donor_resource_identity_drift_is_rejected(self) -> None:
        manifest = json.loads(
            module.DONOR_ROLE_PATH.read_text(encoding="utf-8-sig")
        )
        manifest["sourceResourceRepairs"][0]["sha256"] = "0" * 64
        directory = tempfile.TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        path = pathlib.Path(directory.name) / module.DONOR_ROLE_PATH.name
        path.write_text(json.dumps(manifest), encoding="utf-8")
        with self.assertRaises(module.ArtistGrassTipError):
            module.validate_donor_resources(path)


if __name__ == "__main__":
    unittest.main()
