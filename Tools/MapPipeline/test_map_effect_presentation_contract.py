#!/usr/bin/env python3
"""Executable contract tests for Map Effect cross-document publishing."""

from __future__ import annotations

import copy
import json
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path
from typing import Callable


AREA_ID = "LV_LUT_HEARTRB_ED"
PLACEMENT_ID = "7000000000000000002"
GROUP_ID = "destroyable.group.valtan.floor30.brick.7000000000000000002"
EFFECT_ID = "effect.valtan.environment.red-vortex-sky"
PATTERN_ID = "VALTAN_FOUR_PILLARS_105"
REPO_ROOT = Path(__file__).resolve().parents[2]
PUBLISHER = REPO_ROOT / "Tools" / "MapPipeline" / "Publish-MapAuthoring.ps1"
POWERSHELL = shutil.which("powershell")


class Fixture:
    def __init__(self) -> None:
        self._temp = tempfile.TemporaryDirectory(prefix="LostArkMapEffectContract.")
        self.root = Path(self._temp.name)
        self.authoring = self.root / "Data" / "Maps" / "Authoring" / AREA_ID
        self.imported = self.root / "Data" / "Maps" / "Imported" / AREA_ID
        self.runtime_map = self.root / "Client" / "Bin" / "DataFiles" / "Map"
        self.runtime_world = self.root / "Client" / "Bin" / "DataFiles" / "World"
        self.runtime_resources = self.root / "Client" / "Bin" / "Resources"
        self.effect_authored = self.root / "Data" / "Effects" / "Authored"
        self.encounter_root = self.root / "Data" / "Encounters" / "Valtan"
        for path in (
            self.authoring,
            self.imported,
            self.runtime_map,
            self.runtime_world,
            self.runtime_resources,
            self.effect_authored,
            self.encounter_root,
        ):
            path.mkdir(parents=True, exist_ok=True)
        self._write_fixture()

    def close(self) -> None:
        self._temp.cleanup()

    def _write_json(self, path: Path, value: object) -> None:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(
            json.dumps(value, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )

    @property
    def map_effect_path(self) -> Path:
        return self.authoring / f"{AREA_ID}.mapeffects.json"

    @property
    def deploy_catalog_path(self) -> Path:
        return self.imported / f"{AREA_ID}.deployassets"

    @property
    def deploy_placement_path(self) -> Path:
        return self.authoring / f"{AREA_ID}.deployplacements"

    @property
    def destruction_path(self) -> Path:
        return self.runtime_world / f"{AREA_ID}.worlddestruction.json"

    @property
    def effect_catalog_path(self) -> Path:
        return self.root / "Data" / "Effects" / "EffectCatalog.json"

    @property
    def authored_effect_path(self) -> Path:
        return self.effect_authored / f"{EFFECT_ID}.effect.json"

    @property
    def encounter_path(self) -> Path:
        return self.encounter_root / "ValtanEncounter.json"

    @property
    def world_events_path(self) -> Path:
        return self.encounter_root / "ValtanWorldEvents.json"

    def read_json(self, path: Path) -> dict:
        return json.loads(path.read_text(encoding="utf-8"))

    def write_json(self, path: Path, value: object) -> None:
        self._write_json(path, value)

    def _write_fixture(self) -> None:
        map_catalog = {
            "formatVersion": 1,
            "areas": [
                {
                    "id": AREA_ID,
                    "sourceCatalog": (
                        f"Data/Maps/Imported/{AREA_ID}/{AREA_ID}.mapassets"
                    ),
                    "sourcePlacements": (
                        f"Data/Maps/Authoring/{AREA_ID}/{AREA_ID}.mapplacements"
                    ),
                    "catalog": f"Client/Bin/DataFiles/Map/{AREA_ID}.mapassets",
                    "placements": (
                        f"Client/Bin/DataFiles/Map/{AREA_ID}.mapplacements"
                    ),
                    "sourceLights": (
                        f"Data/Maps/Authoring/{AREA_ID}/{AREA_ID}.maplights.json"
                    ),
                    "lights": f"Client/Bin/DataFiles/Map/{AREA_ID}.maplights.json",
                    "sourceEffects": (
                        f"Data/Maps/Authoring/{AREA_ID}/{AREA_ID}.mapeffects.json"
                    ),
                    "effects": f"Client/Bin/DataFiles/Map/{AREA_ID}.mapeffects.json",
                }
            ],
        }
        self._write_json(self.root / "Data" / "Maps" / "MapCatalog.json", map_catalog)
        (self.imported / f"{AREA_ID}.mapassets").write_text(
            f'LOSTARK_MAP_ASSET_CATALOG 1 "{AREA_ID}" 0\n', encoding="utf-8"
        )
        (self.authoring / f"{AREA_ID}.mapplacements").write_text(
            f'LOSTARK_MAP_PLACEMENTS 2 "{AREA_ID}" 1\n'
            '1 "fixture" "fixture" "editor" "FIXTURE" '
            "0 0 0 0 0 0 1 1 1\n",
            encoding="utf-8",
        )
        self.deploy_catalog_path.write_text(
            f'LOSTARK_DEPLOY_PROP_CATALOG 2 "{AREA_ID}" 1\n'
            '"VALTAN_FLOOR_BRICK_A" STATIC "fixture.floor" '
            '"Map/fixture.wmodel" "Prototype_Fixture" "" "" 1.5 1 '
            '"fixture deferred pass"\n',
            encoding="utf-8",
        )
        self.deploy_placement_path.write_text(
            f'LOSTARK_DEPLOY_PROP_PLACEMENTS 1 "{AREA_ID}" 1\n'
            f'{PLACEMENT_ID} 1 1 "fixture.source" "VALTAN_FLOOR_BRICK_A" '
            "0 0 0 0 0 0 1 1 1 0 0\n",
            encoding="utf-8",
        )
        self._write_json(
            self.authoring / f"{AREA_ID}.maplights.json",
            {
                "schema": "lostark.map-light-presentation",
                "formatVersion": 1,
                "areaId": AREA_ID,
                "provenance": "PROJECT_AUTHORED",
                "lights": [
                    {
                        "lightId": "fixture.light",
                        "sourceLevel": "fixture",
                        "sourceObjectId": "fixture.light.source",
                        "position": [0.0, 1.0, 0.0],
                        "radiusMeters": 1.0,
                        "falloffExponent": 2.0,
                        "color": [1.0, 0.0, 0.0, 1.0],
                        "brightness": 1.0,
                    }
                ],
            },
        )
        self._write_json(
            self.map_effect_path,
            {
                "schema": "lostark.map-effect-presentation",
                "formatVersion": 1,
                "areaId": AREA_ID,
                "presentations": [
                    {
                        "independentEffectId": "fixture.floor",
                        "displayName": "Fixture Floor",
                        "presentationKind": "DEPLOY_SURFACE_OVERLAY",
                        "owners": [
                            {"groupId": GROUP_ID, "placementId": PLACEMENT_ID}
                        ],
                        "visibleStates": ["INTACT"],
                        "materialIndex": 1,
                        "emissiveIntensity": 1.5,
                        "emissiveColor": [0.1, 1.0, 0.5, 1.0],
                        "maskPower": 1.0,
                    },
                    {
                        "independentEffectId": "fixture.sky",
                        "displayName": "Fixture Sky",
                        "presentationKind": "EFFECT_DOCUMENT",
                        "placementId": "fixture.sky.placement",
                        "effectAssetId": EFFECT_ID,
                        "position": [0.0, 10.0, 0.0],
                        "rotationQuaternion": [0.0, 0.0, 0.0, 1.0],
                        "scale": [1.0, 1.0, 1.0],
                        "orientationPolicy": "CAMERA_FACING_WORLD",
                        "activationPolicy": "SERVER_PATTERN_WINDOW",
                        "activationSetId": "fixture.activation",
                        "activationWindows": [
                            {
                                "patternId": PATTERN_ID,
                                "stageId": "TAKEOFF",
                                "effectTimelineOffsetMs": 0,
                            },
                            {
                                "patternId": PATTERN_ID,
                                "stageId": "RECOVERY",
                                "effectTimelineOffsetMs": 100,
                            },
                        ],
                        "playbackPolicy": "SERVER_CLOCK_SAMPLE",
                    },
                ],
            },
        )
        self._write_json(
            self.destruction_path,
            {
                "schema": "lostark.world-destruction-client-projection",
                "formatVersion": 3,
                "areaId": AREA_ID,
                "combatRuntimeRevision": "a" * 64,
                "groups": [
                    {
                        "groupId": GROUP_ID,
                        "mutationId": "fixture.floor.collapse",
                        "removesGround": True,
                        "suppressionAliasPlacementIds": [],
                        "memberPlacementIds": [PLACEMENT_ID],
                    }
                ],
            },
        )
        self._write_json(
            self.world_events_path,
            {
                "schema": "lostark.world-destruction-events",
                "formatVersion": 1,
                "areaId": AREA_ID,
                "encounterId": "ENCOUNTER_VALTAN",
                "provenance": "PROJECT_AUTHORED",
                "groups": [
                    {
                        "groupId": GROUP_ID,
                        "memberPlacementIds": [PLACEMENT_ID],
                        "navigationRegionIds": ["fixture.floor.navregion"],
                        "navPolarity": "BLOCK_WHILE_FRACTURED",
                        "initialState": "INTACT",
                    }
                ],
                "mutations": [
                    {
                        "mutationId": "fixture.floor.collapse",
                        "groupId": GROUP_ID,
                        "targetState": "DESPAWNED",
                        "breakingDurationMs": 100,
                    }
                ],
                "bindings": [],
            },
        )
        self._write_json(
            self.effect_catalog_path,
            {
                "formatVersion": 1,
                "effects": [
                    {
                        "effectAssetId": EFFECT_ID,
                        "payloadKind": "DIRECT_AUTHORED_DOCUMENT",
                        "authoringPath": (
                            f"Effects/Authored/{EFFECT_ID}.effect.json"
                        ),
                    }
                ],
            },
        )
        self._write_json(
            self.authored_effect_path,
            {
                "schema": "lostark.effect-authoring",
                "version": 13,
                "effectAssetId": EFFECT_ID,
                "displayName": "Fixture Sky",
                "particleSystem": {},
                "modelCues": [],
                "elements": [
                    {
                        "id": "fixture.sky.layer",
                        "visible": True,
                        "kind": "particle",
                        "resources": [
                            {
                                "slotId": "base",
                                "assetId": "Effect/Fixture/fixture.dds",
                            }
                        ],
                        "material": {
                            "templateId": "effect.standard",
                            "renderProfile": "alpha_two_sided_depth_read",
                        },
                        "detail": {
                            "timing": {
                                "startDelaySeconds": 0.0,
                                "lifeTimeSeconds": 1.0,
                                "afterImageSeconds": 0.0,
                            }
                        },
                    }
                ],
            },
        )
        resource = self.runtime_resources / "Effect" / "Fixture" / "fixture.dds"
        resource.parent.mkdir(parents=True, exist_ok=True)
        resource.write_bytes(b"DDS " + b"fixture")
        self._write_json(
            self.encounter_path,
            {
                "schema": "lostark.encounter-profile",
                "formatVersion": 4,
                "encounterId": "ENCOUNTER_VALTAN",
                "patterns": [
                    {
                        "patternId": PATTERN_ID,
                        "stages": [
                            {"stageId": "TAKEOFF", "durationMs": 100},
                            {"stageId": "RECOVERY", "durationMs": 200},
                        ],
                    }
                ],
            },
        )

    def snapshot_runtime(self) -> dict[str, bytes]:
        if not self.runtime_map.exists():
            return {}
        return {
            path.relative_to(self.runtime_map).as_posix(): path.read_bytes()
            for path in sorted(self.runtime_map.rglob("*"))
            if path.is_file()
        }

    def publish(self, failure_after_promote: int = 0) -> subprocess.CompletedProcess[str]:
        if POWERSHELL is None:
            raise RuntimeError("powershell executable was not found")
        command = [
            POWERSHELL,
            "-NoProfile",
            "-ExecutionPolicy",
            "Bypass",
            "-File",
            str(PUBLISHER),
            "-AreaId",
            AREA_ID,
            "-ProjectRoot",
            str(self.root),
        ]
        if failure_after_promote:
            command += ["-FailureAfterPromote", str(failure_after_promote)]
        return subprocess.run(
            command,
            cwd=REPO_ROOT,
            text=True,
            encoding="utf-8",
            errors="replace",
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            check=False,
        )


Mutator = Callable[[Fixture], None]


class MapEffectPresentationContractTests(unittest.TestCase):
    def run_negative(self, mutate: Mutator, expected: str) -> None:
        fixture = Fixture()
        try:
            before = fixture.snapshot_runtime()
            mutate(fixture)
            result = fixture.publish()
            self.assertNotEqual(0, result.returncode, result.stdout)
            self.assertIn(expected, result.stdout)
            self.assertEqual(before, fixture.snapshot_runtime())
        finally:
            fixture.close()

    def test_canonical_publish(self) -> None:
        fixture = Fixture()
        try:
            result = fixture.publish()
            self.assertEqual(0, result.returncode, result.stdout)
            runtime = fixture.runtime_map / f"{AREA_ID}.mapeffects.json"
            self.assertTrue(runtime.is_file())
            self.assertEqual(
                fixture.read_json(fixture.map_effect_path), fixture.read_json(runtime)
            )
        finally:
            fixture.close()

    def test_surface_visibility_is_exact_intact_only(self) -> None:
        def mutate(fixture: Fixture) -> None:
            document = fixture.read_json(fixture.map_effect_path)
            document["presentations"][0]["visibleStates"] = ["INTACT", "BREAKING"]
            fixture.write_json(fixture.map_effect_path, document)

        self.run_negative(mutate, "surface values are invalid")

    def test_surface_deploy_and_destruction_cross_joins(self) -> None:
        cases: list[tuple[str, Mutator, str]] = []

        def material_zero(fixture: Fixture) -> None:
            document = fixture.read_json(fixture.map_effect_path)
            document["presentations"][0]["materialIndex"] = 0
            fixture.write_json(fixture.map_effect_path, document)

        cases.append(("material", material_zero, "material-1"))

        def missing_placement(fixture: Fixture) -> None:
            document = fixture.read_json(fixture.map_effect_path)
            document["presentations"][0]["owners"][0]["placementId"] = (
                "7000000000000000009"
            )
            fixture.write_json(fixture.map_effect_path, document)

        cases.append(("placement", missing_placement, "does not resolve to a Deploy"))

        def animated_asset(fixture: Fixture) -> None:
            text = fixture.deploy_catalog_path.read_text(encoding="utf-8")
            fixture.deploy_catalog_path.write_text(
                text.replace('"VALTAN_FLOOR_BRICK_A" STATIC', '"VALTAN_FLOOR_BRICK_A" ANIM'),
                encoding="utf-8",
            )

        cases.append(("static", animated_asset, "static destructible"))

        def no_deferred_pass(fixture: Fixture) -> None:
            text = fixture.deploy_catalog_path.read_text(encoding="utf-8")
            fixture.deploy_catalog_path.write_text(
                text.replace('"" "" 1.5 1 "fixture', '"" "" 1.5 0 "fixture'),
                encoding="utf-8",
            )

        cases.append(("deferred", no_deferred_pass, "deferred-emissive"))

        def not_destructible(fixture: Fixture) -> None:
            text = fixture.deploy_placement_path.read_text(encoding="utf-8")
            fixture.deploy_placement_path.write_text(
                text.replace("0 0 0 0 0 0 1 1 1 0 0", "0 0 0 0 0 0 1 1 0 0 0"),
                encoding="utf-8",
            )

        cases.append(("destructible", not_destructible, "static destructible"))

        def missing_group(fixture: Fixture) -> None:
            document = fixture.read_json(fixture.map_effect_path)
            document["presentations"][0]["owners"][0]["groupId"] = "fixture.missing"
            fixture.write_json(fixture.map_effect_path, document)

        cases.append(("group", missing_group, "unknown destruction group"))

        def missing_membership(fixture: Fixture) -> None:
            document = fixture.read_json(fixture.destruction_path)
            document["groups"][0]["memberPlacementIds"] = ["7000000000000000009"]
            fixture.write_json(fixture.destruction_path, document)

        cases.append(("membership", missing_membership, "must be a member"))

        def keeps_ground(fixture: Fixture) -> None:
            document = fixture.read_json(fixture.destruction_path)
            document["groups"][0]["removesGround"] = False
            fixture.write_json(fixture.destruction_path, document)

        cases.append(("removesGround", keeps_ground, "removesGround"))

        def stale_projection(fixture: Fixture) -> None:
            document = fixture.read_json(fixture.world_events_path)
            document["mutations"][0]["mutationId"] = "fixture.floor.changed"
            fixture.write_json(fixture.world_events_path, document)

        cases.append(("stale-source", stale_projection, "stale against its current source"))

        for name, mutate, expected in cases:
            with self.subTest(name=name):
                self.run_negative(mutate, expected)

    def test_direct_authored_effect_identity_cross_join(self) -> None:
        cases: list[tuple[str, Mutator, str]] = []

        def unknown_effect(fixture: Fixture) -> None:
            document = fixture.read_json(fixture.effect_catalog_path)
            document["effects"] = []
            fixture.write_json(fixture.effect_catalog_path, document)

        cases.append(("catalog", unknown_effect, "resolve exactly once"))

        def not_direct(fixture: Fixture) -> None:
            document = fixture.read_json(fixture.effect_catalog_path)
            document["effects"][0]["payloadKind"] = "SOURCE_RECIPE"
            fixture.write_json(fixture.effect_catalog_path, document)

        cases.append(("payload", not_direct, "direct-authored"))

        def wrong_identity(fixture: Fixture) -> None:
            document = fixture.read_json(fixture.authored_effect_path)
            document["effectAssetId"] = "effect.fixture.wrong"
            fixture.write_json(fixture.authored_effect_path, document)

        cases.append(("identity", wrong_identity, "authored identity"))

        def non_drawable(fixture: Fixture) -> None:
            document = fixture.read_json(fixture.authored_effect_path)
            document["elements"][0]["visible"] = False
            fixture.write_json(fixture.authored_effect_path, document)

        cases.append(("drawable", non_drawable, "no visible drawable"))

        def malformed_material(fixture: Fixture) -> None:
            document = fixture.read_json(fixture.authored_effect_path)
            del document["elements"][0]["material"]
            fixture.write_json(fixture.authored_effect_path, document)

        cases.append(("material", malformed_material, "identity/material/detail"))

        def malformed_detail(fixture: Fixture) -> None:
            document = fixture.read_json(fixture.authored_effect_path)
            del document["elements"][0]["detail"]["timing"]
            fixture.write_json(fixture.authored_effect_path, document)

        cases.append(("detail", malformed_detail, "no finite timing"))

        def unsafe_resource(fixture: Fixture) -> None:
            document = fixture.read_json(fixture.authored_effect_path)
            document["elements"][0]["resources"][0]["assetId"] = "../escape.dds"
            fixture.write_json(fixture.authored_effect_path, document)

        cases.append(("resource-path", unsafe_resource, "unsafe or unsupported"))

        def missing_resource(fixture: Fixture) -> None:
            document = fixture.read_json(fixture.authored_effect_path)
            document["elements"][0]["resources"][0]["assetId"] = (
                "Effect/Fixture/missing.dds"
            )
            fixture.write_json(fixture.authored_effect_path, document)

        cases.append(("resource-file", missing_resource, "resource file is missing"))

        for name, mutate, expected in cases:
            with self.subTest(name=name):
                self.run_negative(mutate, expected)

    def test_pattern_stage_window_exact_offset_and_duration_join(self) -> None:
        cases: list[tuple[str, Mutator, str]] = []

        def wrong_offset(fixture: Fixture) -> None:
            document = fixture.read_json(fixture.map_effect_path)
            document["presentations"][1]["activationWindows"][1][
                "effectTimelineOffsetMs"
            ] = 101
            fixture.write_json(fixture.map_effect_path, document)

        cases.append(("offset", wrong_offset, "stage offset/duration"))

        def missing_stage(fixture: Fixture) -> None:
            document = fixture.read_json(fixture.map_effect_path)
            document["presentations"][1]["activationWindows"] = document[
                "presentations"
            ][1]["activationWindows"][:1]
            fixture.write_json(fixture.map_effect_path, document)

        cases.append(("coverage", missing_stage, "exactly cover every pattern stage"))

        def wrong_stage(fixture: Fixture) -> None:
            document = fixture.read_json(fixture.map_effect_path)
            document["presentations"][1]["activationWindows"][1]["stageId"] = "WRONG"
            fixture.write_json(fixture.map_effect_path, document)

        cases.append(("stage", wrong_stage, "stage offset/duration"))

        def too_short(fixture: Fixture) -> None:
            document = fixture.read_json(fixture.encounter_path)
            document["patterns"][0]["stages"][1]["durationMs"] = 1000
            fixture.write_json(fixture.encounter_path, document)

        cases.append(("duration", too_short, "duration does not cover"))

        for name, mutate, expected in cases:
            with self.subTest(name=name):
                self.run_negative(mutate, expected)

    def test_transaction_rolls_back_after_promote_failure(self) -> None:
        fixture = Fixture()
        try:
            first = fixture.publish()
            self.assertEqual(0, first.returncode, first.stdout)
            before = fixture.snapshot_runtime()
            document = fixture.read_json(fixture.map_effect_path)
            document["presentations"][0]["emissiveIntensity"] = 2.0
            fixture.write_json(fixture.map_effect_path, document)
            failed = fixture.publish(failure_after_promote=1)
            self.assertNotEqual(0, failed.returncode, failed.stdout)
            self.assertIn("Injected map publish failure", failed.stdout)
            self.assertEqual(before, fixture.snapshot_runtime())
        finally:
            fixture.close()


if __name__ == "__main__":
    unittest.main(verbosity=2)
