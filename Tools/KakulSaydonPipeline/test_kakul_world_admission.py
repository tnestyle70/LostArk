#!/usr/bin/env python3
"""Executable contracts for the fail-closed KakulSaydon admission gate."""

from __future__ import annotations

import json
import contextlib
import io
from pathlib import Path
import struct
import sys
import tempfile
import unittest


SCRIPT_DIR = Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

from validate_kakul_world_admission import (  # noqa: E402
    AdmissionReport,
    AREA_ID,
    COLLECTION_NAME,
    Finding,
    GEOMETRY_MODE,
    ModeResult,
    PRODUCT_MODE,
    RESOURCE_MODE,
    WORLD_ID,
    format_human,
    main,
    validate_repository,
)


class Fixture:
    def __init__(self) -> None:
        self._temp = tempfile.TemporaryDirectory(prefix="lostark-kakul-admission-")
        self.root = Path(self._temp.name)

    def close(self) -> None:
        self._temp.cleanup()

    def write_text(self, relative: str, value: str) -> None:
        path = self.root / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(value, encoding="utf-8", newline="\n")

    def write_bytes(self, relative: str, value: bytes) -> None:
        path = self.root / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(value)

    def write_json(self, relative: str, value: object) -> None:
        self.write_text(
            relative,
            json.dumps(value, ensure_ascii=False, indent=2) + "\n",
        )

    def read_json(self, relative: str) -> dict:
        return json.loads((self.root / relative).read_text(encoding="utf-8"))

    def install_resource_collection(self) -> None:
        self.write_json(
            f"Data/ResourceIntake/{AREA_ID}.resource-intake.json",
            {
                "schema": "lostark.resource-intake",
                "formatVersion": 2,
                "intakeId": "raid.fixture.kakul",
                "aliasContract": {
                    "alias": COLLECTION_NAME,
                    "kind": "user-folder-alias",
                    "canonical": False,
                    "canonicalAreaId": AREA_ID,
                },
                "canonicalIdentity": {"areaId": AREA_ID},
            },
        )
        self.write_bytes(
            f"Client/Bin/Resources/Map/{AREA_ID}/FIXTURE/FIXTURE.wmodel",
            b"fixture-wmodel",
        )

    def install_geometry(self, *, kind: str = "development") -> None:
        self.install_resource_collection()
        self.write_json(
            "Data/Maps/MapCatalog.json",
            {
                "schema": "lostark.map-catalog",
                "formatVersion": 1,
                "areas": [
                    {
                        "id": AREA_ID,
                        "kind": kind,
                        "catalogType": "single",
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
                        "placementCount": 1,
                        "assetCount": 1,
                        "runtimeAssetRoot": f"Map/{AREA_ID}",
                    }
                ],
            },
        )
        self.write_text(
            f"Data/Maps/Imported/{AREA_ID}/{AREA_ID}.mapassets",
            f'LOSTARK_MAP_ASSET_CATALOG 4 "{AREA_ID}" 1\n'
            f'"FIXTURE" "Fixture" "Map/{AREA_ID}/FIXTURE/FIXTURE.wmodel" '
            '"Prototype_Component_Model_FIXTURE" 1 1 1 Origin\n',
        )
        self.write_text(
            f"Data/Maps/Authoring/{AREA_ID}/{AREA_ID}.mapplacements",
            f'LOSTARK_MAP_PLACEMENTS 2 "{AREA_ID}" 1\n'
            '1 "fixture:1" "fixture" "actor" "FIXTURE" '
            "0 0 0 0 0 0 1 1 1 1 1\n",
        )

    @staticmethod
    def navgrid(*, blocked_index: int | None = None) -> bytes:
        width, height = 2, 2
        walkable = bytearray((1, 1, 1, 1))
        if blocked_index is not None:
            walkable[blocked_index] = 0
        return (
            struct.pack("<IIfff", width, height, 1.0, 0.0, 0.0)
            + bytes(walkable)
            + struct.pack("<4f", 0.0, 0.0, 0.0, 0.0)
        )

    def install_product(self) -> None:
        self.install_geometry(kind="product")
        catalog_path = "Data/Maps/MapCatalog.json"
        catalog = self.read_json(catalog_path)
        catalog["areas"][0].update(
            {
                "navigationSource": f"Data/Navigation/{AREA_ID}.navsource",
                "navigationPaint": f"Data/Navigation/{AREA_ID}.navpaint",
                "navigationRuntime": (
                    f"Client/Bin/DataFiles/Navigation/{AREA_ID}.navgrid"
                ),
                "gameplayDocument": f"Data/Worlds/{AREA_ID}/Gameplay.world.json",
            }
        )
        self.write_json(catalog_path, catalog)

        source_catalog = (
            self.root
            / f"Data/Maps/Imported/{AREA_ID}/{AREA_ID}.mapassets"
        ).read_bytes()
        source_placements = (
            self.root
            / f"Data/Maps/Authoring/{AREA_ID}/{AREA_ID}.mapplacements"
        ).read_bytes()
        self.write_bytes(
            f"Client/Bin/DataFiles/Map/{AREA_ID}.mapassets", source_catalog
        )
        self.write_bytes(
            f"Client/Bin/DataFiles/Map/{AREA_ID}.mapplacements", source_placements
        )

        self.write_json(
            f"Data/Worlds/{AREA_ID}/Gameplay.world.json",
            {
                "schema": "lostark.world-gameplay",
                "formatVersion": 6,
                "areaId": AREA_ID,
                "revision": 1,
                "placements": [
                    {
                        "placementId": "player.spawn.kakul.slot01",
                        "kind": "playerSpawn",
                        "archetypeId": None,
                        "encounterId": None,
                        "position": [0.25, 0.0, 0.25],
                        "yawDegrees": 0.0,
                        "enabled": True,
                    }
                ],
            },
        )
        self.write_text(
            f"Data/Navigation/{AREA_ID}.navsource",
            f'LOSTARK_NAVGRID_SOURCE 3 "{AREA_ID}" 2 2 1 0 0 4\n',
        )
        self.write_text(
            f"Data/Navigation/{AREA_ID}.navpaint",
            f'LOSTARK_NAVGRID_PAINT 3 "{AREA_ID}" 2 2 1 0 0 0\n',
        )
        grid = self.navgrid()
        for output in ("Server/Bin/DataFiles", "Client/Bin/DataFiles"):
            self.write_bytes(f"{output}/Navigation/{AREA_ID}.navgrid", grid)
            self.write_text(
                f"{output}/Navigation/{AREA_ID}.navpolicy",
                f"LOSTARK_NAVIGATION_POLICY 1 {AREA_ID}\n",
            )
            self.write_text(
                f"{output}/Navigation/{AREA_ID}.navblockers",
                f"LOSTARK_NAVIGATION_BLOCKERS 1 {AREA_ID} 0\n",
            )
        self.write_text(
            "Tools/NavigationPipeline/Publish-ServerNavigation.ps1",
            "Convert-NavigationAuthoringGrid `\n"
            f" -RelativeSourcePath 'Data/Navigation/{AREA_ID}.navsource' `\n"
            f" -RelativePaintPath 'Data/Navigation/{AREA_ID}.navpaint'\n",
        )
        self.write_text(
            "Tools/WorldPipeline/Publish-WorldGameplay.ps1",
            f"(Convert-WorldDocument -AreaId '{AREA_ID}' -WorldId '{WORLD_ID}')\n",
        )

        self.write_text(
            "Shared/Public/Network/PacketType.h",
            "enum class WORLD_ID {\n"
            f" {WORLD_ID} = 5,\n END\n}};\n"
            "constexpr bool Is_Known_World_Id(WORLD_ID worldId) {\n"
            f" return WORLD_ID::{WORLD_ID} == worldId;\n}}\n",
        )
        self.write_text(
            "Server/Private/WorldBootstrap.cpp",
            f'case WORLD_ID::{WORLD_ID}: return "{WORLD_ID}";\n',
        )
        self.write_text(
            "Server/Private/ServerApp.cpp",
            f"stageSharedSimulation(WORLD_ID::{WORLD_ID});\n",
        )
        self.write_text(
            f"Server/Bin/DataFiles/World/{WORLD_ID}.worldbootstrap",
            f"LOSTARK_WORLD_BOOTSTRAP\t7\t{WORLD_ID}\t1\n",
        )

        self.write_text(
            "Client/Public/Client_Defines.h",
            f"enum class LEVEL {{ {CLIENT_LEVEL_TOKEN}, END }};\n",
        )
        self.write_text(
            "Client/Private/LevelRegistry.cpp",
            "CreateKakulSaydonArena\n"
            f"LEVEL::{CLIENT_LEVEL_TOKEN}\n"
            "CLIENT_LEVEL_KIND::PRODUCT\n"
            '"raid.kakul-saydon.arena"\n'
            f'"{AREA_ID}"\n'
            "Ready_For_KakulSaydonArena\n",
        )
        self.write_text(
            "Client/Public/Loader.h", "HRESULT Ready_For_KakulSaydonArena();\n"
        )
        self.write_text(
            "Client/Private/Loader.cpp",
            f"Ready_For_KakulSaydonArena LEVEL::{CLIENT_LEVEL_TOKEN}\n",
        )
        self.write_text(
            "Client/Private/LevelTransitionService.cpp",
            f"WORLD_ID::{WORLD_ID} LEVEL::{CLIENT_LEVEL_TOKEN}\n",
        )
        self.write_text(
            "Client/Public/Level_KakulSaydonArena.h",
            "class CLevel_KakulSaydonArena {};\n",
        )
        self.write_text(
            "Client/Private/Level_KakulSaydonArena.cpp",
            f"CLevel_KakulSaydonArena {AREA_ID}\n",
        )
        self.write_text(
            "Client/Default/Client.vcxproj",
            "Level_KakulSaydonArena.h\nLevel_KakulSaydonArena.cpp\n",
        )
        self.write_text(
            "Client/Default/Client.vcxproj.filters",
            "Level_KakulSaydonArena.h\nLevel_KakulSaydonArena.cpp\n",
        )

        sound_id = f"Sound/{COLLECTION_NAME}/KAKUL_ENTER.wav"
        self.write_json(
            "Data/Sound/CharacterSoundCatalog.json",
            {
                "formatVersion": 1,
                "classes": {COLLECTION_NAME: {"KAKUL_ENTER": [sound_id]}},
            },
        )
        self.write_bytes(f"Client/Bin/Resources/{sound_id}", b"RIFF-fixture")


CLIENT_LEVEL_TOKEN = "KAKULSAYDON_ARENA"


class KakulWorldAdmissionTests(unittest.TestCase):
    def setUp(self) -> None:
        self.fixture = Fixture()

    def tearDown(self) -> None:
        self.fixture.close()

    def test_resource_collection_does_not_imply_geometry_or_product(self) -> None:
        self.fixture.install_resource_collection()
        report = validate_repository(self.fixture.root)
        self.assertTrue(report.result(RESOURCE_MODE).permitted)
        self.assertFalse(report.result(GEOMETRY_MODE).permitted)
        self.assertFalse(report.result(PRODUCT_MODE).permitted)
        self.assertEqual(RESOURCE_MODE, report.highest_permitted_mode)
        geometry_codes = {
            finding.code for finding in report.result(GEOMETRY_MODE).own_findings
        }
        self.assertIn("file.missing", geometry_codes)

    def test_development_geometry_preserves_stable_area_identity(self) -> None:
        self.fixture.install_geometry(kind="development")
        report = validate_repository(self.fixture.root)
        self.assertTrue(report.result(RESOURCE_MODE).permitted)
        self.assertTrue(report.result(GEOMETRY_MODE).permitted)
        self.assertFalse(report.result(PRODUCT_MODE).permitted)
        self.assertEqual(GEOMETRY_MODE, report.highest_permitted_mode)
        product_codes = {
            finding.code for finding in report.result(PRODUCT_MODE).own_findings
        }
        self.assertIn("map-catalog.product-kind.missing", product_codes)
        self.assertIn("file.missing", product_codes)

    def test_complete_fixture_admits_server_product_level(self) -> None:
        self.fixture.install_product()
        report = validate_repository(self.fixture.root)
        self.assertTrue(all(mode.permitted for mode in report.modes))
        self.assertEqual(PRODUCT_MODE, report.highest_permitted_mode)
        self.assertEqual(1, report.result(PRODUCT_MODE).facts["stablePlayerSpawnCount"])
        self.assertEqual(1, report.result(PRODUCT_MODE).facts["playableSoundAssetCount"])
        payload = report.as_json(PRODUCT_MODE)
        self.assertEqual(AREA_ID, payload["canonicalAreaId"])
        self.assertEqual(COLLECTION_NAME, payload["collectionName"])
        self.assertEqual("forbidden", payload["spawnContract"]["arbitraryRandomWorldPosition"])

    def test_random_or_non_walkable_spawn_is_never_admitted(self) -> None:
        self.fixture.install_product()
        world_path = f"Data/Worlds/{AREA_ID}/Gameplay.world.json"
        world = self.fixture.read_json(world_path)
        world["spawnPolicy"] = "random-world-position"
        world["placements"][0]["position"] = [1.25, 0.0, 0.25]
        self.fixture.write_json(world_path, world)
        blocked_grid = self.fixture.navgrid(blocked_index=1)
        for output in ("Server/Bin/DataFiles", "Client/Bin/DataFiles"):
            self.fixture.write_bytes(
                f"{output}/Navigation/{AREA_ID}.navgrid", blocked_grid
            )

        report = validate_repository(self.fixture.root)
        self.assertTrue(report.result(GEOMETRY_MODE).permitted)
        self.assertFalse(report.result(PRODUCT_MODE).permitted)
        findings = report.result(PRODUCT_MODE).own_findings
        codes = {finding.code for finding in findings}
        self.assertIn("world.random-spawn.forbidden", codes)
        self.assertIn("navigation.spawn.blocked", codes)

    def test_wrong_area_header_blocks_geometry_with_exact_finding(self) -> None:
        self.fixture.install_geometry()
        path = f"Data/Maps/Imported/{AREA_ID}/{AREA_ID}.mapassets"
        text = (self.fixture.root / path).read_text(encoding="utf-8")
        self.fixture.write_text(path, text.replace(AREA_ID, "WRONG_AREA", 1))
        report = validate_repository(self.fixture.root)
        self.assertFalse(report.result(GEOMETRY_MODE).permitted)
        findings = report.result(GEOMETRY_MODE).own_findings
        self.assertTrue(
            any(
                finding.code == "map-assets.header.identity"
                and finding.path.endswith(f"{AREA_ID}.mapassets")
                for finding in findings
            )
        )

    def test_cli_defaults_to_fail_closed_product_gate(self) -> None:
        self.fixture.install_resource_collection()
        with contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(1, main(["--repo-root", str(self.fixture.root)]))
            self.assertEqual(
                0,
                main(
                    [
                        "--repo-root",
                        str(self.fixture.root),
                        "--require",
                        RESOURCE_MODE,
                        "--json",
                    ]
                ),
            )
            self.assertEqual(
                0,
                main(
                    [
                        "--repo-root",
                        str(self.fixture.root),
                        "--report-only",
                    ]
                ),
            )

    def test_human_report_is_bounded_but_json_and_verbose_keep_all(self) -> None:
        findings = [
            Finding(
                "geometry.resource.missing",
                f"Client/Bin/Resources/Map/{AREA_ID}/fixture-{index}.wmodel",
                f"fixture-{index}",
            )
            for index in range(20)
        ]
        report = AdmissionReport(
            self.fixture.root,
            [
                ModeResult(RESOURCE_MODE, permitted=True),
                ModeResult(GEOMETRY_MODE, own_findings=findings, permitted=False),
                ModeResult(PRODUCT_MODE, permitted=False),
            ],
        )

        bounded = format_human(report, PRODUCT_MODE)
        self.assertIn("geometry.resource.missing=20", bounded)
        self.assertIn("... 8 more finding(s)", bounded)
        self.assertIn("fixture-11.wmodel", bounded)
        self.assertNotIn("fixture-12.wmodel", bounded)

        verbose = format_human(report, PRODUCT_MODE, verbose=True)
        self.assertIn("fixture-19.wmodel", verbose)
        self.assertNotIn("more finding(s)", verbose)

        payload = report.as_json(PRODUCT_MODE)
        geometry = next(
            mode for mode in payload["modes"] if mode["mode"] == GEOMETRY_MODE
        )
        self.assertEqual(20, len(geometry["findings"]))


if __name__ == "__main__":
    unittest.main()
