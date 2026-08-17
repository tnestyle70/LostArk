#!/usr/bin/env python3
"""Regression contract for the generated Valtan stage Effect slice."""

from __future__ import annotations

import contextlib
import importlib.util
import io
import json
from pathlib import Path
import sys
import unittest
from unittest import mock


SCRIPT_PATH = Path(__file__).resolve().with_name(
    "build_valtan_stage_effects.py"
)
SPEC = importlib.util.spec_from_file_location(
    "build_valtan_stage_effects", SCRIPT_PATH
)
assert SPEC is not None and SPEC.loader is not None
builder = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(builder)

REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
DATA_ROOT = REPOSITORY_ROOT / "Data"
AUTHORED_ROOT = DATA_ROOT / "Effects" / "Authored"
ENCOUNTER_PATH = (
    DATA_ROOT / "Encounters" / "Valtan" / "ValtanEncounter.json"
)
CUE_PATH = (
    DATA_ROOT
    / "Animation"
    / "Authored"
    / "Valtan"
    / "Valtan.patterneffectcues.json"
)
CATALOG_PATH = DATA_ROOT / "Effects" / "EffectCatalog.json"

CANARY_ASSET_ID = "effect.valtan.pattern.420633.active"

# These stages have animation bindings but no source emitter with a usable
# mesh or texture. They are deliberately absent instead of receiving an empty
# Effect document. Pin the identities, not only the count, so one accidental
# addition cannot conceal one accidental removal.
INTENTIONALLY_SILENT_STAGES = frozenset(
    {
        ("VALTAN_ARENA_BREAK_109", "DROP"),
        ("VALTAN_ARENA_BREAK_33", "LANDING"),
        ("VALTAN_ARENA_BREAK_33", "SPIN"),
        ("VALTAN_ARMOR_BREAK_OPENING", "GROGGY"),
        ("VALTAN_ARMOR_BREAK_OPENING", "RECOVERY"),
        ("VALTAN_BACKSTEP_ATTACK", "SWEEP"),
        ("VALTAN_BIND_CHARGE_SMASH", "RECOVERY"),
        ("VALTAN_CENTER_GRAB_COUNTER_64", "TARGET_EXPLOSION"),
        ("VALTAN_CHARGE_GRAB_ROAR", "CHARGE"),
        ("VALTAN_ENTRANCE_WHIRLWIND", "WINDUP"),
        ("VALTAN_FIST_IN_OUT", "WINDUP"),
        ("VALTAN_FLOOR_WIPE_130", "RECOVERY"),
        ("VALTAN_FOUR_PILLARS_105", "RECOVERY"),
        ("VALTAN_FOUR_PILLARS_105", "YELLOW_ZONE"),
        ("VALTAN_GHOST_TRANSITION_15", "OUTER"),
        ("VALTAN_GROUND_WAVE_SMASH", "RECOVERY"),
        ("VALTAN_HIGH_JUMP", "AIRBORNE"),
        ("VALTAN_JUMP_SPIN", "LAND"),
        ("VALTAN_MAGIC_ORB_STAGGER_76", "RECOVERY"),
        ("VALTAN_SUPER_SMASH", "IMPACTS"),
        ("VALTAN_TRIPLE_COUNTER", "RECOVERY"),
        ("VALTAN_WHIRLWIND", "WINDUP"),
    }
)

FLOOR_WIPE_CUES = {
    "WINDUP": {
        "bindingId": "cue.valtan.floor-wipe-130.six-direction-telegraph",
        "effectAssetId": "effect.valtan.floor-wipe-130.windup",
        "displayName": "발탄 / 115줄 / 6방향 공격 예고",
        "semantic": "telegraph",
    },
    "FIRST_SMASH": {
        "bindingId": "cue.valtan.floor-wipe-130.six-direction-impact",
        "effectAssetId": "effect.valtan.floor-wipe-130.first-smash",
        "displayName": "발탄 / 115줄 / 6방향 공격 충격",
        "semantic": "impact",
    },
    "INTERVAL": {
        "bindingId": "cue.valtan.floor-wipe-130.arena-wipe-telegraph",
        "effectAssetId": "effect.valtan.floor-wipe-130.interval",
        "displayName": "발탄 / 115줄 / 전멸 공격 예고",
        "semantic": "telegraph",
    },
    "SECOND_SMASH": {
        "bindingId": "cue.valtan.floor-wipe-130.arena-wipe-impact",
        "effectAssetId": "effect.valtan.floor-wipe-130.second-smash",
        "displayName": "발탄 / 115줄 / 전멸 공격 충격",
        "semantic": "impact",
    },
}


def _read_json(path: Path) -> dict[str, object]:
    return json.loads(path.read_text(encoding="utf-8-sig"))


class ValtanStageEffectSeedTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.encounter = _read_json(ENCOUNTER_PATH)
        cls.cue_document = _read_json(CUE_PATH)
        cls.catalog = _read_json(CATALOG_PATH)

        cls.captured_writes: dict[Path, object] = {}

        def capture_write(path: str, value: object) -> None:
            resolved = Path(path).resolve()
            if resolved in cls.captured_writes:
                raise AssertionError(f"generator wrote the same path twice: {resolved}")
            cls.captured_writes[resolved] = value

        output = io.StringIO()
        with mock.patch.object(
            sys, "argv", [str(SCRIPT_PATH), "--write"]
        ), mock.patch.object(
            builder, "write_json_atomic", side_effect=capture_write
        ), contextlib.redirect_stdout(output):
            cls.generator_result = builder.main()
        cls.generator_output = output.getvalue()

        cls.patterns = cls.encounter["patterns"]
        cls.cues = cls.cue_document["cues"]
        cls.stages = [
            (pattern, stage)
            for pattern in cls.patterns
            for stage in pattern["stages"]
        ]
        cls.stage_by_key = {
            (pattern["patternId"], stage["stageId"]): stage
            for pattern, stage in cls.stages
        }
        cls.cue_by_stage = {
            (cue["patternId"], cue["stageId"]): cue
            for cue in cls.cues
        }

    def test_generator_projection_matches_the_checked_in_generated_set(self) -> None:
        self.assertEqual(self.generator_result, 0)
        for expected_line in (
            "stages                 121",
            "stages without a clip  0",
            "stages without source  22",
            "documents              99",
        ):
            self.assertIn(expected_line + "\n", self.generator_output)

        generated_document_paths = {
            path
            for path in self.captured_writes
            if path.parent == AUTHORED_ROOT.resolve()
            and path.name.startswith("effect.valtan.")
            and path.name.endswith(".effect.json")
        }
        self.assertEqual(len(generated_document_paths), 99)
        self.assertEqual(len(self.captured_writes), 101)
        self.assertIn(CUE_PATH.resolve(), self.captured_writes)
        self.assertIn(CATALOG_PATH.resolve(), self.captured_writes)

        for path in generated_document_paths:
            self.assertTrue(path.is_file(), f"missing generated document: {path}")
            self.assertEqual(_read_json(path), self.captured_writes[path])
        self.assertEqual(
            self.cue_document, self.captured_writes[CUE_PATH.resolve()]
        )
        self.assertEqual(self.catalog, self.captured_writes[CATALOG_PATH.resolve()])

    def test_121_stages_partition_into_99_exact_cues_and_22_silent_stages(
        self,
    ) -> None:
        self.assertEqual(len(self.patterns), 32)
        self.assertEqual(len(self.stages), 121)
        self.assertEqual(len(self.stage_by_key), 121)

        encounter_action_ids = [stage["actionId"] for _, stage in self.stages]
        self.assertEqual(len(set(encounter_action_ids)), 121)

        self.assertEqual(
            self.cue_document,
            {
                "schema": "lostark.valtan-pattern-effect-cues",
                "formatVersion": 1,
                "ownerArchetypeId": "BOSS_VALTAN",
                "cues": self.cues,
            },
        )
        self.assertEqual(len(self.cues), 99)
        self.assertEqual(len(self.cue_by_stage), 99)

        for field in ("bindingId", "actionId", "effectAssetId"):
            values = [cue[field] for cue in self.cues]
            self.assertEqual(
                len(values), len(set(values)), f"duplicate cue {field}"
            )

        for cue in self.cues:
            key = (cue["patternId"], cue["stageId"])
            self.assertIn(key, self.stage_by_key)
            stage = self.stage_by_key[key]
            self.assertEqual(cue["actionId"], stage["actionId"])
            self.assertEqual(cue["startMs"], 0)
            self.assertGreater(cue["endMs"], cue["startMs"])
            self.assertLessEqual(cue["endMs"], stage["durationMs"])
            self.assertEqual(cue["endMs"], stage["durationMs"])
            self.assertEqual(cue["anchorSlotId"], "root")
            self.assertEqual(cue["followPolicy"], "follow")
            self.assertEqual(cue["stopPolicy"], "cue_end")
            self.assertEqual(
                cue["localTransform"],
                {
                    "position": [0, 0, 0],
                    "rotationDegrees": [0, 0, 0],
                    "scale": [1, 1, 1],
                },
            )

        silent = set(self.stage_by_key) - set(self.cue_by_stage)
        self.assertEqual(silent, INTENTIONALLY_SILENT_STAGES)
        self.assertEqual(len(silent), 22)

    def test_seeded_documents_and_catalog_are_exact_and_exclude_the_canary(
        self,
    ) -> None:
        cue_effect_ids = {cue["effectAssetId"] for cue in self.cues}
        self.assertEqual(len(cue_effect_ids), 99)
        self.assertNotIn(CANARY_ASSET_ID, cue_effect_ids)

        authored_ids = {
            path.name[: -len(".effect.json")]
            for path in AUTHORED_ROOT.glob("effect.valtan.*.effect.json")
        }
        # Every cue needs its document and the canary must survive, but the
        # Effect Tool's Create Effect legitimately adds authoring-only drafts
        # for the silent stages. Those may exist; they may not reach the cue
        # set or the catalog, which is the invariant this pins.
        self.assertTrue(cue_effect_ids | {CANARY_ASSET_ID} <= authored_ids)
        self.assertTrue(
            (AUTHORED_ROOT / f"{CANARY_ASSET_ID}.effect.json").is_file()
        )
        authoring_only_ids = authored_ids - cue_effect_ids - {CANARY_ASSET_ID}

        catalog_ids = [row["effectAssetId"] for row in self.catalog["effects"]]
        self.assertEqual(len(catalog_ids), len(set(catalog_ids)))
        valtan_rows = {
            row["effectAssetId"]: row
            for row in self.catalog["effects"]
            if row["effectAssetId"].startswith("effect.valtan.")
        }
        self.assertEqual(set(valtan_rows), cue_effect_ids)
        self.assertNotIn(CANARY_ASSET_ID, valtan_rows)
        for authoring_only in sorted(authoring_only_ids):
            self.assertNotIn(authoring_only, valtan_rows)

        for effect_id in sorted(cue_effect_ids):
            expected_path = f"Effects/Authored/{effect_id}.effect.json"
            self.assertEqual(
                valtan_rows[effect_id],
                {
                    "effectAssetId": effect_id,
                    "payloadKind": "DIRECT_AUTHORED_DOCUMENT_V13",
                    "authoringPath": expected_path,
                },
            )
            document_path = DATA_ROOT / expected_path
            document = _read_json(document_path)
            self.assertEqual(document["schema"], "lostark.effect-authoring")
            self.assertEqual(document["version"], 13)
            self.assertEqual(document["effectAssetId"], effect_id)
            self.assertTrue(document["elements"])

    def test_floor_wipe_names_and_guides_match_the_server_geometry(self) -> None:
        floor_cues = {
            cue["stageId"]: cue
            for cue in self.cues
            if cue["patternId"] == "VALTAN_FLOOR_WIPE_130"
        }
        self.assertEqual(set(floor_cues), set(FLOOR_WIPE_CUES))

        first_smash = self.stage_by_key[
            ("VALTAN_FLOOR_WIPE_130", "FIRST_SMASH")
        ]
        second_smash = self.stage_by_key[
            ("VALTAN_FLOOR_WIPE_130", "SECOND_SMASH")
        ]
        self.assertEqual(first_smash["hitShape"], "SIX_DIRECTIONS")
        self.assertEqual(first_smash["hitLength"], 14.0)
        self.assertEqual(first_smash["hitHalfWidth"], 2.2)
        self.assertEqual(second_smash["hitShape"], "CIRCLE")
        self.assertEqual(second_smash["hitOuterRadius"], 100.0)

        for stage_id, contract in FLOOR_WIPE_CUES.items():
            cue = floor_cues[stage_id]
            self.assertEqual(cue["bindingId"], contract["bindingId"])
            self.assertEqual(cue["effectAssetId"], contract["effectAssetId"])
            document = _read_json(
                AUTHORED_ROOT / f"{cue['effectAssetId']}.effect.json"
            )
            self.assertEqual(document["displayName"], contract["displayName"])

            duration_seconds = round(
                self.stage_by_key[
                    ("VALTAN_FLOOR_WIPE_130", stage_id)
                ]["durationMs"]
                / 1000.0,
                3,
            )
            if stage_id in ("WINDUP", "FIRST_SMASH"):
                self._assert_six_direction_guides(
                    document, contract["semantic"], duration_seconds,
                    first_smash,
                )
            else:
                self._assert_arena_wipe_guide(
                    document, contract["semantic"], duration_seconds,
                    second_smash,
                )

    def _assert_six_direction_guides(
        self,
        document: dict[str, object],
        semantic: str,
        duration_seconds: float,
        hit_stage: dict[str, object],
    ) -> None:
        guides = {
            element["id"]: element
            for element in document["elements"]
            if element["id"].startswith("six-direction-")
        }
        expected_ids = {
            f"six-direction-{semantic}.axis-{angle:03d}"
            for angle in (0, 60, 120)
        }
        self.assertEqual(set(guides), expected_ids)
        self.assertFalse(
            any(
                element["id"].startswith("arena-wipe-")
                for element in document["elements"]
            )
        )

        expected_size = [
            hit_stage["hitHalfWidth"] * 2.0,
            hit_stage["hitLength"] * 2.0,
        ]
        for angle in (0, 60, 120):
            guide = guides[
                f"six-direction-{semantic}.axis-{angle:03d}"
            ]
            self.assertEqual(guide["kind"], "decal")
            self.assertEqual(guide["groupId"], f"six_direction_{semantic}")
            self.assertEqual(
                guide["sourceNode"],
                "project-authored:valtan.floor-wipe-130."
                f"{semantic}.axis-{angle:03d}",
            )
            self.assertEqual(
                guide["resources"],
                [
                    {
                        "slotId": "base",
                        "assetId": (
                            "Effect/Valtan/Textures/FX_TEX_00/"
                            "fx_a_line_010_ycl.dds"
                        ),
                    }
                ],
            )
            transform = guide["detail"]["transform"]
            self.assertEqual(transform["position"], [0, 0.08, 0])
            self.assertEqual(transform["rotationDegrees"], [0, angle, 0])
            self.assertEqual(transform["scale"], [1, 1, 1])
            self.assertEqual(guide["detail"]["decal"]["size"], expected_size)
            self.assertEqual(guide["detail"]["decal"]["depth"], 1.0)
            self.assertEqual(
                guide["detail"]["timing"]["lifeTimeSeconds"],
                duration_seconds,
            )
            self.assertIs(guide["sourceRecipe"]["enabled"], False)
            self.assertIs(guide["sourcePresentation"]["enabled"], False)

    def _assert_arena_wipe_guide(
        self,
        document: dict[str, object],
        semantic: str,
        duration_seconds: float,
        hit_stage: dict[str, object],
    ) -> None:
        guides = [
            element
            for element in document["elements"]
            if element["id"].startswith("arena-wipe-")
        ]
        self.assertEqual(len(guides), 1)
        self.assertFalse(
            any(
                element["id"].startswith("six-direction-")
                for element in document["elements"]
            )
        )
        guide = guides[0]
        self.assertEqual(guide["id"], f"arena-wipe-{semantic}.radius-100")
        self.assertEqual(guide["kind"], "decal")
        self.assertEqual(guide["groupId"], f"arena_wipe_{semantic}")
        self.assertEqual(
            guide["sourceNode"],
            "project-authored:valtan.floor-wipe-130."
            f"{semantic}.radius-100",
        )
        self.assertEqual(
            guide["resources"],
            [
                {
                    "slotId": "base",
                    "assetId": (
                        "Effect/Valtan/Textures/FX_TEX_04/"
                        "fx_f_ring_001.dds"
                    ),
                }
            ],
        )
        transform = guide["detail"]["transform"]
        self.assertEqual(transform["position"], [0, 0.06, 0])
        self.assertEqual(transform["rotationDegrees"], [0, 0, 0])
        self.assertEqual(transform["scale"], [1, 1, 1])
        diameter = hit_stage["hitOuterRadius"] * 2.0
        self.assertEqual(guide["detail"]["decal"]["size"], [diameter, diameter])
        self.assertEqual(guide["detail"]["decal"]["depth"], 2.0)
        self.assertEqual(
            guide["detail"]["timing"]["lifeTimeSeconds"], duration_seconds
        )
        self.assertIs(guide["sourceRecipe"]["enabled"], False)
        self.assertIs(guide["sourcePresentation"]["enabled"], False)

    def test_blend_mode_follows_the_original_material_name_suffix(
        self,
    ) -> None:
        """An _ad material is additive here exactly as it is for the classes.

        Flattening every row to alpha is what made the boss read dark against
        the four player classes, so both the generator and the checked-in
        documents are pinned to the same rule.
        """
        self.assertEqual(
            builder.resolve_render_profile("fx_m_mi_01.fx_mi.fx_d_pa_gl_01_2_ad"),
            "additive_two_sided_depth_read",
        )
        self.assertEqual(
            builder.resolve_render_profile("fx_m_mi_01.fx_mi.fx_d_pa_gl_01_2_tr"),
            "alpha_two_sided_depth_read",
        )
        self.assertEqual(
            builder.resolve_render_profile(""), "alpha_two_sided_depth_read"
        )

        additive = 0
        for effect_id in sorted(
            cue["effectAssetId"] for cue in self.cues
        ):
            document = _read_json(
                AUTHORED_ROOT / f"{effect_id}.effect.json"
            )
            for element in document["elements"]:
                material = element["material"]
                expected = builder.resolve_render_profile(
                    material.get("sourceMaterialPath") or ""
                )
                self.assertEqual(
                    material["renderProfile"],
                    expected,
                    f"{effect_id}/{element['id']}",
                )
                if expected == "additive_two_sided_depth_read":
                    additive += 1
        self.assertEqual(additive, 1198)

    def test_mesh_carriers_use_the_centimetre_to_metre_pre_scale(self) -> None:
        """Cascade meshes are centimetre-authored; this runtime is metres.

        Without the 0.01 factor a mesh carrier renders a hundred times too
        large. transform.scale stays at 1.0 because that is the value a person
        edits in Effect Detail.
        """
        carriers = 0
        for effect_id in sorted(
            cue["effectAssetId"] for cue in self.cues
        ):
            document = _read_json(
                AUTHORED_ROOT / f"{effect_id}.effect.json"
            )
            for element in document["elements"]:
                mesh_block = element["detail"]["mesh"]
                if builder.element_carries_mesh(element):
                    carriers += 1
                    self.assertEqual(
                        mesh_block.get("modelPreScale"),
                        0.01,
                        f"{effect_id}/{element['id']}",
                    )
                else:
                    self.assertNotIn(
                        "modelPreScale",
                        mesh_block,
                        f"{effect_id}/{element['id']}",
                    )
        self.assertEqual(carriers, 1940)

    def test_sprite_particles_billboard_and_mesh_particles_do_not(self) -> None:
        """The renderer only faces a quad at the camera when this flag is set.

        A sprite particle without it keeps one world orientation and disappears
        edge-on, which is the camera-angle dropout the boss showed. A mesh
        carrier has real geometry and must keep its own orientation. The four
        player classes split exactly this way with no exception.
        """
        sprites = 0
        meshes = 0
        for effect_id in sorted(
            cue["effectAssetId"] for cue in self.cues
        ):
            document = _read_json(
                AUTHORED_ROOT / f"{effect_id}.effect.json"
            )
            for element in document["elements"]:
                if element["kind"] != "particle":
                    continue
                billboard = element["detail"]["particle"]["billboard"]
                if builder.element_carries_mesh(element):
                    meshes += 1
                    self.assertIs(
                        billboard, False, f"{effect_id}/{element['id']}"
                    )
                else:
                    sprites += 1
                    self.assertIs(
                        billboard, True, f"{effect_id}/{element['id']}"
                    )
        self.assertEqual(meshes, 1940)
        self.assertEqual(sprites, 1158)


if __name__ == "__main__":
    unittest.main()
