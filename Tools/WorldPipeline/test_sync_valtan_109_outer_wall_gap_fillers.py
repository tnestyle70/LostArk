from __future__ import annotations

import json
import math
import tempfile
import unittest
from pathlib import Path

from sync_valtan_109_outer_wall_gap_fillers import (
    ANGLE_STEP_DEGREES,
    ARENA_CENTER_X,
    ARENA_CENTER_Z,
    ARENA_RADIUS,
    ARENA_Y,
    AREA_ID,
    FILLER_ANGLE_OFFSET_DEGREES,
    FILLER_ASSET_ID,
    FILLER_ID_BASE,
    FILLER_PREFIX,
    FILLER_SCALE,
    GROUP_PREFIX,
    OutOfSyncError,
    SOURCE_ASSET_ID,
    SOURCE_COUNT,
    SOURCE_ID_BASE,
    SOURCE_PREFIX,
    SyncError,
    SyncPaths,
    parse_deploy_document,
    synchronize,
)


SOURCE_ANGLE_BY_SUFFIX = {
    **{suffix: float((suffix - 1) * 12) for suffix in range(1, 12)},
    **{suffix: float(180 + (suffix - 12) * 12) for suffix in range(12, 21)},
    **{suffix: float(312 + (suffix - 21) * 12) for suffix in range(21, 25)},
    **{suffix: float(132 + (suffix - 25) * 12) for suffix in range(25, 29)},
    29: 288.0,
    30: 300.0,
}


def quaternion(angle_degrees: float) -> tuple[float, float, float, float]:
    yaw = math.radians(90.0 - angle_degrees)
    return (0.0, math.sin(yaw * 0.5), 0.0, math.cos(yaw * 0.5))


def polar_angle(x: float, z: float) -> float:
    return math.degrees(math.atan2(z - ARENA_CENTER_Z, x - ARENA_CENTER_X)) % 360.0


def angular_delta(left: float, right: float) -> float:
    return (left - right) % 360.0


class GapFillerFixture:
    newline = "\r\n"

    def __init__(self, root: Path):
        self.root = root
        self.paths = SyncPaths(
            deploy_placements=root / "arena.deployplacements",
            world_events=root / "ValtanWorldEvents.json",
            destruction_simulation=root / "arena.destructionsimulation.json",
        )
        self.write_all()

    @staticmethod
    def source_row(suffix: int) -> str:
        runtime_id = SOURCE_ID_BASE + suffix
        angle = SOURCE_ANGLE_BY_SUFFIX[suffix]
        radians = math.radians(angle)
        x = ARENA_CENTER_X + ARENA_RADIUS * math.cos(radians)
        z = ARENA_CENTER_Z + ARENA_RADIUS * math.sin(radians)
        q = quaternion(angle)
        return (
            f'{runtime_id} 109000001 109000002 "{SOURCE_PREFIX}{runtime_id}" '
            f'"{SOURCE_ASSET_ID}" {x:.6f} {ARENA_Y:.6f} {z:.6f} '
            f"{q[0]:.9f} {q[1]:.9f} {q[2]:.9f} {q[3]:.9f} 1 1 0 0"
        )

    @staticmethod
    def unrelated_row() -> str:
        return (
            '42 7 8 "fixture:unrelated" "UNRELATED_ASSET" '
            "1 2 3 0 0 0 1 1 0 0 0"
        )

    @staticmethod
    def json_text(value: dict) -> str:
        return (json.dumps(value, ensure_ascii=False, indent=2) + "\n").replace(
            "\n", GapFillerFixture.newline
        )

    def events(self) -> dict:
        return {
            "schema": "lostark.world-destruction-events",
            "formatVersion": 1,
            "areaId": AREA_ID,
            "encounterId": "ENCOUNTER_VALTAN",
            "provenance": "fixture",
            "groups": [
                {
                    "groupId": "destroyable.group.fixture.unrelated",
                    "memberPlacementIds": ["42"],
                    "navigationRegionIds": [],
                    "navPolarity": "BLOCK_WHILE_INTACT",
                    "initialState": "INTACT",
                },
                *[
                    {
                        "groupId": GROUP_PREFIX + str(SOURCE_ID_BASE + suffix),
                        "memberPlacementIds": [str(SOURCE_ID_BASE + suffix)],
                        "navigationRegionIds": [
                            "navregion.valtan.outerwall109."
                            + str(SOURCE_ID_BASE + suffix)
                        ],
                        "navPolarity": "BLOCK_WHILE_INTACT",
                        "initialState": "INTACT",
                    }
                    for suffix in range(1, SOURCE_COUNT + 1)
                ],
            ],
            "mutations": [],
            "bindings": [],
        }

    def simulation(self) -> dict:
        def element(source_id: str) -> dict:
            return {
                "elementId": "debris." + source_id,
                "sourceRuntimePlacementId": source_id,
                "suppressionAliasPlacementIds": [],
                "spawnOffset": [0, 0, 0],
                "direction": [1, 0.3, 0],
                "speedMetersPerSecond": 6,
                "gravityScale": 2,
                "lifetimeSeconds": 4,
                "trigger": {
                    "kind": "TIMELINE_TIME",
                    "timeSeconds": 0,
                    "receiverCollisionId": "",
                },
            }

        return {
            "schema": "lostark.destruction-simulation",
            "formatVersion": 2,
            "areaId": AREA_ID,
            "profiles": [
                {
                    "profileId": "destroyable.group.fixture.unrelated.preview",
                    "groupId": "destroyable.group.fixture.unrelated",
                    "durationSeconds": 5,
                    "previewGroundEnabled": True,
                    "previewGroundHeight": 20,
                    "previewGroundHalfExtents": [30, 30],
                    "elements": [element("42")],
                },
                *[
                    {
                        "profileId": GROUP_PREFIX
                        + str(SOURCE_ID_BASE + suffix)
                        + ".preview",
                        "groupId": GROUP_PREFIX + str(SOURCE_ID_BASE + suffix),
                        "durationSeconds": 5,
                        "previewGroundEnabled": True,
                        "previewGroundHeight": 20,
                        "previewGroundHalfExtents": [30, 30],
                        "elements": [element(str(SOURCE_ID_BASE + suffix))],
                    }
                    for suffix in range(1, SOURCE_COUNT + 1)
                ],
            ],
        }

    def write_all(self) -> None:
        self.root.mkdir(parents=True, exist_ok=True)
        rows = [self.unrelated_row()] + [
            self.source_row(suffix) for suffix in range(1, SOURCE_COUNT + 1)
        ]
        self.paths.deploy_placements.write_bytes(
            (
                f'LOSTARK_DEPLOY_PROP_PLACEMENTS 1 "{AREA_ID}" {len(rows)}'
                + self.newline
                + self.newline.join(rows)
                + self.newline
            ).encode("utf-8")
        )
        self.paths.world_events.write_bytes(self.json_text(self.events()).encode("utf-8"))
        self.paths.destruction_simulation.write_bytes(
            self.json_text(self.simulation()).encode("utf-8")
        )

    def output_bytes(self) -> dict[Path, bytes]:
        return {
            path: path.read_bytes()
            for path in (
                self.paths.deploy_placements,
                self.paths.world_events,
                self.paths.destruction_simulation,
            )
        }


class GapFillerSyncTests(unittest.TestCase):
    def test_sync_is_complete_suffix_paired_idempotent_and_checkable(self):
        with tempfile.TemporaryDirectory() as temporary:
            fixture = GapFillerFixture(Path(temporary))
            before = fixture.output_bytes()
            changed = synchronize(fixture.paths)
            self.assertEqual(set(changed), set(before))

            deploy = parse_deploy_document(fixture.paths.deploy_placements)
            self.assertEqual(deploy.newline, "\r\n")
            self.assertEqual(len(deploy.rows), 1 + SOURCE_COUNT * 2)
            self.assertEqual(deploy.rows[0].original_line, fixture.unrelated_row())
            by_id = {row.runtime_id: row for row in deploy.rows}
            filler_angles = []
            for suffix in range(1, SOURCE_COUNT + 1):
                source = by_id[SOURCE_ID_BASE + suffix]
                filler_id = FILLER_ID_BASE + suffix
                filler = by_id[filler_id]
                source_angle = polar_angle(source.position[0], source.position[2])
                filler_angle = polar_angle(filler.position[0], filler.position[2])
                filler_angles.append(round(filler_angle))
                self.assertAlmostEqual(
                    angular_delta(filler_angle, source_angle),
                    FILLER_ANGLE_OFFSET_DEGREES,
                    places=4,
                )
                self.assertEqual(filler.source_id, FILLER_PREFIX + str(filler_id))
                self.assertEqual(filler.asset_id, FILLER_ASSET_ID)
                self.assertAlmostEqual(filler.uniform_scale, FILLER_SCALE)
                self.assertEqual(filler.destructible, 1)
            self.assertEqual(
                sorted(filler_angles),
                [int(FILLER_ANGLE_OFFSET_DEGREES + ANGLE_STEP_DEGREES * index) for index in range(30)],
            )

            events = json.loads(fixture.paths.world_events.read_text(encoding="utf-8"))
            event_groups = {group["groupId"]: group for group in events["groups"]}
            simulation = json.loads(
                fixture.paths.destruction_simulation.read_text(encoding="utf-8")
            )
            profiles = {profile["groupId"]: profile for profile in simulation["profiles"]}
            for suffix in range(1, SOURCE_COUNT + 1):
                primary = str(SOURCE_ID_BASE + suffix)
                filler = str(FILLER_ID_BASE + suffix)
                group_id = GROUP_PREFIX + primary
                self.assertEqual(
                    event_groups[group_id]["memberPlacementIds"], [primary, filler]
                )
                self.assertEqual(
                    profiles[group_id]["elements"][0]["sourceRuntimePlacementId"],
                    primary,
                )
                self.assertEqual(
                    profiles[group_id]["elements"][0]["suppressionAliasPlacementIds"],
                    [filler],
                )

            synchronized = fixture.output_bytes()
            self.assertEqual(synchronize(fixture.paths, check_only=True), ())
            self.assertEqual(synchronize(fixture.paths), ())
            self.assertEqual(fixture.output_bytes(), synchronized)

    def test_check_only_reports_all_stale_files_without_writing(self):
        with tempfile.TemporaryDirectory() as temporary:
            fixture = GapFillerFixture(Path(temporary))
            before = fixture.output_bytes()
            with self.assertRaises(OutOfSyncError) as raised:
                synchronize(fixture.paths, check_only=True)
            self.assertEqual(set(raised.exception.paths), set(before))
            self.assertEqual(fixture.output_bytes(), before)

    def test_malformed_json_fails_before_any_write(self):
        with tempfile.TemporaryDirectory() as temporary:
            fixture = GapFillerFixture(Path(temporary))
            fixture.paths.destruction_simulation.write_bytes(b"{\r\n")
            before = fixture.output_bytes()
            with self.assertRaisesRegex(SyncError, "JSON is malformed"):
                synchronize(fixture.paths)
            self.assertEqual(fixture.output_bytes(), before)

    def test_filler_runtime_id_collision_fails_before_any_write(self):
        with tempfile.TemporaryDirectory() as temporary:
            fixture = GapFillerFixture(Path(temporary))
            text = fixture.paths.deploy_placements.read_text(encoding="utf-8")
            lines = text.splitlines()
            lines[0] = f'LOSTARK_DEPLOY_PROP_PLACEMENTS 1 "{AREA_ID}" {len(lines)}'
            lines.append(
                f'{FILLER_ID_BASE + 1} 91 92 "fixture:foreign" "FOREIGN" '
                "1 2 3 0 0 0 1 1 1 0 0"
            )
            fixture.paths.deploy_placements.write_bytes(
                (fixture.newline.join(lines) + fixture.newline).encode("utf-8")
            )
            before = fixture.output_bytes()
            with self.assertRaisesRegex(SyncError, "occupied by an unrelated"):
                synchronize(fixture.paths)
            self.assertEqual(fixture.output_bytes(), before)

    def test_source_angle_grid_drift_fails_before_any_write(self):
        with tempfile.TemporaryDirectory() as temporary:
            fixture = GapFillerFixture(Path(temporary))
            text = fixture.paths.deploy_placements.read_text(encoding="utf-8")
            source_id = SOURCE_ID_BASE + 2
            canonical = fixture.source_row(2)
            fields = canonical.split(" ")
            fields[5] = f"{ARENA_CENTER_X + ARENA_RADIUS:.6f}"
            fields[7] = f"{ARENA_CENTER_Z:.6f}"
            drifted = " ".join(fields)
            fixture.paths.deploy_placements.write_text(
                text.replace(canonical, drifted), encoding="utf-8", newline=""
            )
            before = fixture.output_bytes()
            with self.assertRaisesRegex(SyncError, "source .* drifted|angles do not cover"):
                synchronize(fixture.paths)
            self.assertEqual(fixture.output_bytes(), before)

    def test_injected_mid_commit_failure_restores_all_three_files(self):
        with tempfile.TemporaryDirectory() as temporary:
            fixture = GapFillerFixture(Path(temporary))
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
