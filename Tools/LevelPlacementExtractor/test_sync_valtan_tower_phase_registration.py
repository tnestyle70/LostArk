from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path

from build_maptool_scene import IMPORTED_ID_BIT, imported_id
from sync_valtan_tower_phase_registration import (
    OutOfSyncError,
    SyncError,
    SyncPaths,
    parse_placement_document,
    synchronize,
)


AREA_ID = "LV_LUT_HEARTRB_ED"
SOURCE_LEVEL = "LV_LUT_HEARTRB_ED_SL04"
OVERLAY_LEVEL = "VALTAN_TOWER_REGISTERED"
TRANSLATION_Y = 11.0


class TowerRegistrationFixture:
    def __init__(self, root: Path):
        self.root = root
        self.paths = SyncPaths(
            manifest=root / "registration.json",
            placements=root / "arena.mapplacements",
            overlay_manifest=root / "overlay.json",
            environment_runtime=root / "environment.json",
            maplights=root / "maplights.json",
        )
        self.rear_sources = [
            [f"source:rear:{station}:{index}" for index in range(47)]
            for station in range(4)
        ]
        self.control_sources = [f"source:control:{index}" for index in range(47)]
        self.rear_light_ids = [f"light.rear.{index}" for index in range(4)]
        self.control_light_id = "light.control"
        self.overlay_starts = [1001, 1101, 1201, 1301]
        self.write_all()

    @staticmethod
    def write_json(path: Path, value: dict) -> None:
        path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")

    @staticmethod
    def source_line(
        source_id: str,
        source_level: str,
        *,
        y: float,
        transform_source: str = "component",
        asset_id: str = "ASSET_A",
        visible: bool = True,
        placement_id: int | None = None,
    ) -> str:
        actual_id = imported_id(source_id) if placement_id is None else placement_id
        return (
            f'{actual_id} {json.dumps(source_id)} {json.dumps(source_level)} '
            f'{json.dumps(transform_source)} {json.dumps(asset_id)} '
            f"1 {y:.9g} -2 0 0 0 1 1 1 1 {1 if visible else 0}"
        )

    @staticmethod
    def stale_overlay() -> dict:
        return {
            "placementId": 900,
            "sourcePlacementId": "overlay:stale-core",
            "sourceLevel": "VALTAN_CORE_INTACT",
            "transformSource": "overlay",
            "assetId": "STALE_ASSET",
            "position": [0.0, 0.0, 0.0],
            "quaternion": [0.0, 0.0, 0.0, 1.0],
            "scale": [1.0, 1.0, 1.0],
            "visible": True,
        }

    @staticmethod
    def light(light_id: str, source_position: list[float], source_object: str) -> dict:
        return {
            "lightId": light_id,
            "sourceLevel": SOURCE_LEVEL,
            "sourceObjectId": source_object,
            "position": source_position,
            "radiusMeters": 9.0,
            "falloffExponent": 2.0,
            "color": [1.0, 0.1, 0.0, 1.0],
            "brightness": 6.0,
        }

    def manifest(self) -> dict:
        return {
            "schemaVersion": 1,
            "enabled": True,
            "areaId": AREA_ID,
            "sourceLevel": SOURCE_LEVEL,
            "sourceFloor": {"placementId": "source:floor", "y": 12.0},
            "targetFloor": {"placementId": "target:floor", "y": 23.0},
            "translationY": TRANSLATION_Y,
            "previousTranslationY": TRANSLATION_Y,
            "overlay": {
                "sourceLevel": OVERLAY_LEVEL,
                "sourcePrefix": "tower-registration",
            },
            "expectedAssetCounts": {"ASSET_A": 47},
            "expectedRearStationCount": 4,
            "expectedComponentsPerStation": 47,
            "rearStations": [
                {
                    "stationId": f"rear_{index}",
                    "overlayPlacementIdStart": self.overlay_starts[index],
                    "sourcePlacementIds": self.rear_sources[index],
                    "light": {
                        "lightId": self.rear_light_ids[index],
                        "sourcePosition": [float(index), 24.0, -float(index)],
                    },
                }
                for index in range(4)
            ],
            "controlStation": {
                "stationId": "control",
                "sourcePlacementIds": self.control_sources,
                "light": {
                    "lightId": self.control_light_id,
                    "sourcePosition": [9.0, 24.0, -9.0],
                },
            },
        }

    def write_all(self) -> None:
        self.root.mkdir(parents=True, exist_ok=True)
        self.write_json(self.paths.manifest, self.manifest())
        rows = [
            self.source_line(
                "source:floor", SOURCE_LEVEL, y=12.0, transform_source="actor", asset_id="FLOOR"
            ),
            self.source_line(
                "target:floor", "LV_LUT_HEARTRB_ED_SL00", y=23.0, transform_source="actor", asset_id="FLOOR"
            ),
        ]
        for station_sources in self.rear_sources:
            rows.extend(self.source_line(source, SOURCE_LEVEL, y=13.0) for source in station_sources)
        rows.extend(self.source_line(source, SOURCE_LEVEL, y=13.0) for source in self.control_sources)
        self.paths.placements.write_text(
            f'LOSTARK_MAP_PLACEMENTS 2 "{AREA_ID}" {len(rows)}\n' + "\n".join(rows) + "\n",
            encoding="utf-8",
        )
        self.write_json(
            self.paths.overlay_manifest,
            {
                "schemaVersion": 1,
                "areaId": AREA_ID,
                "status": "fixture",
                "basis": "fixture",
                "assets": [],
                "placements": [self.stale_overlay()],
            },
        )
        self.write_json(
            self.paths.environment_runtime,
            {
                "schemaVersion": 1,
                "areaId": AREA_ID,
                "evidence": "fixture",
                "profiles": [],
                "visibilityOverrides": [
                    {"sourcePlacementId": "source:unrelated", "visible": True}
                ],
            },
        )
        lights = [
            self.light(
                self.rear_light_ids[index],
                [float(index), 24.0, -float(index)],
                f"source-light:{index}",
            )
            for index in range(4)
        ]
        lights.append(
            self.light(self.control_light_id, [9.0, 24.0, -9.0], "source-light:control")
        )
        lights.append(self.light("light.unrelated", [20.0, 10.0, -20.0], "source-light:other"))
        self.write_json(
            self.paths.maplights,
            {
                "schema": "lostark.map-light-presentation",
                "formatVersion": 1,
                "areaId": AREA_ID,
                "provenance": "SOURCE_INSTANCE_EXACT_FALLOFF_INFERRED",
                "lights": lights,
            },
        )

    def output_bytes(self) -> dict[Path, bytes]:
        return {
            path: path.read_bytes()
            for path in (
                self.paths.placements,
                self.paths.overlay_manifest,
                self.paths.environment_runtime,
                self.paths.maplights,
            )
        }


class TowerRegistrationSyncTests(unittest.TestCase):
    def test_sync_is_complete_idempotent_and_checkable(self):
        with tempfile.TemporaryDirectory() as temporary:
            fixture = TowerRegistrationFixture(Path(temporary))
            changed = synchronize(fixture.paths)
            self.assertEqual(
                set(changed),
                {
                    fixture.paths.placements,
                    fixture.paths.overlay_manifest,
                    fixture.paths.environment_runtime,
                    fixture.paths.maplights,
                },
            )

            placement_document = parse_placement_document(fixture.paths.placements, AREA_ID)
            by_source = {row.source_placement_id: row for row in placement_document.rows}
            rear_sources = {source for station in fixture.rear_sources for source in station}
            self.assertEqual(len(rear_sources), 188)
            self.assertTrue(all(not by_source[source].visible for source in rear_sources))
            self.assertTrue(all(by_source[source].visible for source in fixture.control_sources))

            registered = [row for row in placement_document.rows if row.source_level == OVERLAY_LEVEL]
            self.assertEqual(len(registered), 188)
            self.assertTrue(all(row.transform_source == "overlay" and row.visible for row in registered))
            for station_index, station_sources in enumerate(fixture.rear_sources):
                for offset, source in enumerate(station_sources):
                    generated_id = fixture.overlay_starts[station_index] + offset
                    overlay = next(row for row in registered if row.placement_id == generated_id)
                    original = by_source[source]
                    self.assertEqual(overlay.asset_id, original.asset_id)
                    self.assertEqual(overlay.quaternion, original.quaternion)
                    self.assertEqual(overlay.scale, original.scale)
                    self.assertAlmostEqual(overlay.position[0], original.position[0])
                    self.assertAlmostEqual(overlay.position[1], original.position[1] + TRANSLATION_Y)
                    self.assertAlmostEqual(overlay.position[2], original.position[2])

            overlay_manifest = json.loads(fixture.paths.overlay_manifest.read_text(encoding="utf-8"))
            self.assertEqual(overlay_manifest["placements"][0], fixture.stale_overlay())
            self.assertEqual(len(overlay_manifest["placements"]), 189)
            environment = json.loads(fixture.paths.environment_runtime.read_text(encoding="utf-8"))
            self.assertEqual(environment["visibilityOverrides"][0]["sourcePlacementId"], "source:unrelated")
            self.assertEqual(len(environment["visibilityOverrides"]), 189)
            self.assertTrue(
                all(
                    row["visible"] is False
                    for row in environment["visibilityOverrides"][1:]
                )
            )
            maplights = json.loads(fixture.paths.maplights.read_text(encoding="utf-8"))
            self.assertEqual(maplights["provenance"], "PROJECT_AUTHORED")
            lights = {light["lightId"]: light for light in maplights["lights"]}
            for index, light_id in enumerate(fixture.rear_light_ids):
                self.assertEqual(lights[light_id]["position"], [float(index), 35.0, -float(index)])
            self.assertEqual(lights[fixture.control_light_id]["position"], [9.0, 24.0, -9.0])

            synchronized = fixture.output_bytes()
            self.assertEqual(synchronize(fixture.paths, check_only=True), ())
            self.assertEqual(synchronize(fixture.paths), ())
            self.assertEqual(fixture.output_bytes(), synchronized)

    def test_disabled_registration_restores_source_attachment_without_overlays(self):
        with tempfile.TemporaryDirectory() as temporary:
            fixture = TowerRegistrationFixture(Path(temporary))
            synchronize(fixture.paths)

            manifest = fixture.manifest()
            manifest["enabled"] = False
            manifest["targetFloor"] = dict(manifest["sourceFloor"])
            manifest["translationY"] = 0.0
            fixture.write_json(fixture.paths.manifest, manifest)

            changed = synchronize(fixture.paths)
            self.assertEqual(
                set(changed),
                {
                    fixture.paths.placements,
                    fixture.paths.overlay_manifest,
                    fixture.paths.environment_runtime,
                    fixture.paths.maplights,
                },
            )

            placement_document = parse_placement_document(fixture.paths.placements, AREA_ID)
            by_source = {row.source_placement_id: row for row in placement_document.rows}
            rear_sources = {source for station in fixture.rear_sources for source in station}
            self.assertTrue(all(by_source[source].visible for source in rear_sources))
            self.assertFalse(any(row.source_level == OVERLAY_LEVEL for row in placement_document.rows))

            overlay_manifest = json.loads(
                fixture.paths.overlay_manifest.read_text(encoding="utf-8")
            )
            self.assertEqual(overlay_manifest["placements"], [fixture.stale_overlay()])
            environment = json.loads(
                fixture.paths.environment_runtime.read_text(encoding="utf-8")
            )
            self.assertEqual(
                environment["visibilityOverrides"],
                [{"sourcePlacementId": "source:unrelated", "visible": True}],
            )
            maplights = json.loads(fixture.paths.maplights.read_text(encoding="utf-8"))
            self.assertEqual(
                maplights["provenance"],
                "SOURCE_INSTANCE_EXACT_FALLOFF_INFERRED",
            )
            lights = {light["lightId"]: light for light in maplights["lights"]}
            for index, light_id in enumerate(fixture.rear_light_ids):
                self.assertEqual(
                    lights[light_id]["position"],
                    [float(index), 24.0, -float(index)],
                )
            self.assertEqual(synchronize(fixture.paths, check_only=True), ())
            self.assertEqual(synchronize(fixture.paths), ())

    def test_check_only_reports_all_stale_outputs_without_writing(self):
        with tempfile.TemporaryDirectory() as temporary:
            fixture = TowerRegistrationFixture(Path(temporary))
            before = fixture.output_bytes()
            with self.assertRaises(OutOfSyncError) as raised:
                synchronize(fixture.paths, check_only=True)
            self.assertEqual(set(raised.exception.paths), set(before))
            self.assertEqual(fixture.output_bytes(), before)

    def test_imported_stable_id_drift_fails_before_writes(self):
        with tempfile.TemporaryDirectory() as temporary:
            fixture = TowerRegistrationFixture(Path(temporary))
            source = fixture.rear_sources[0][0]
            expected = str(imported_id(source))
            replacement = str((imported_id(source) + 1) | IMPORTED_ID_BIT)
            text = fixture.paths.placements.read_text(encoding="utf-8")
            fixture.paths.placements.write_text(text.replace(expected, replacement, 1), encoding="utf-8")
            before = fixture.output_bytes()
            with self.assertRaisesRegex(SyncError, "stable imported ID mismatch"):
                synchronize(fixture.paths)
            self.assertEqual(fixture.output_bytes(), before)

    def test_shared_manifest_source_fails_before_writes(self):
        with tempfile.TemporaryDirectory() as temporary:
            fixture = TowerRegistrationFixture(Path(temporary))
            manifest = json.loads(fixture.paths.manifest.read_text(encoding="utf-8"))
            manifest["rearStations"][1]["sourcePlacementIds"][0] = (
                manifest["rearStations"][0]["sourcePlacementIds"][0]
            )
            fixture.write_json(fixture.paths.manifest, manifest)
            before = fixture.output_bytes()
            with self.assertRaisesRegex(SyncError, "shared between stations"):
                synchronize(fixture.paths)
            self.assertEqual(fixture.output_bytes(), before)

    def test_control_station_visibility_and_light_are_immutable(self):
        with tempfile.TemporaryDirectory() as temporary:
            fixture = TowerRegistrationFixture(Path(temporary))
            control = fixture.control_sources[0]
            lines = fixture.paths.placements.read_text(encoding="utf-8").splitlines()
            lines = [line[:-1] + "0" if control in line else line for line in lines]
            fixture.paths.placements.write_text("\n".join(lines) + "\n", encoding="utf-8")
            before = fixture.output_bytes()
            with self.assertRaisesRegex(SyncError, "control station source must remain visible"):
                synchronize(fixture.paths)
            self.assertEqual(fixture.output_bytes(), before)

        with tempfile.TemporaryDirectory() as temporary:
            fixture = TowerRegistrationFixture(Path(temporary))
            maplights = json.loads(fixture.paths.maplights.read_text(encoding="utf-8"))
            control = next(
                light for light in maplights["lights"] if light["lightId"] == fixture.control_light_id
            )
            control["position"][1] += 1.0
            fixture.write_json(fixture.paths.maplights, maplights)
            before = fixture.output_bytes()
            with self.assertRaisesRegex(SyncError, "control station light position"):
                synchronize(fixture.paths)
            self.assertEqual(fixture.output_bytes(), before)

    def test_injected_mid_commit_failure_restores_all_four_files(self):
        with tempfile.TemporaryDirectory() as temporary:
            fixture = TowerRegistrationFixture(Path(temporary))
            before = fixture.output_bytes()
            with self.assertRaisesRegex(SyncError, "rolled back"):
                synchronize(fixture.paths, failure_after_promote=2)
            self.assertEqual(fixture.output_bytes(), before)
            leftovers = [
                path
                for path in fixture.root.iterdir()
                if path.suffix in (".stage", ".backup")
            ]
            self.assertEqual(leftovers, [])


if __name__ == "__main__":
    unittest.main()
