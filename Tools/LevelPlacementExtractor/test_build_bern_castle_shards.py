import argparse
import json
import shlex
import tempfile
import unittest
from pathlib import Path

from build_bern_castle_shards import (
    AREA_ID,
    SHARD_IDS,
    SL_SHARD_IDS,
    ShardBuildError,
    build_shards,
    parse_catalog,
    parse_placements,
    shard_file_name,
    validate_relative_filename,
)
from build_maptool_scene import imported_id


class BernCastleShardBuilderTests(unittest.TestCase):
    @staticmethod
    def write_json(path: Path, value) -> None:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(value), encoding="utf-8")

    @staticmethod
    def static_asset(asset_id: str) -> dict:
        return {
            "assetId": asset_id,
            "objectName": asset_id,
            "fullPath": f"bern.mesh.{asset_id}",
            "sourceCategory": "staticmesh",
            "logicalPackage": "BERN",
        }

    @staticmethod
    def source_placement(
        source_id: str,
        asset_id: str,
        level: str,
        scale=(1.0, 1.0, 1.0),
    ) -> dict:
        return {
            "placementId": source_id,
            "levelPackage": level,
            "asset": {"objectPath": f"bern.mesh.{asset_id}"},
            "transform": {
                "source": "actor",
                "position": {"x": 0.0, "y": 0.0, "z": 0.0},
                "rotation": {"pitch": 0, "yaw": 0, "roll": 0},
                "scale3D": {"x": scale[0], "y": scale[1], "z": scale[2]},
            },
        }

    @staticmethod
    def landscape_catalog_row(asset_id: str) -> str:
        values = (
            json.dumps(asset_id),
            json.dumps(f"Landscape {asset_id}"),
            json.dumps(f"Map/Landscape/{asset_id}.wmodel"),
            json.dumps(f"Prototype_Component_Model_{asset_id}"),
            "1 1 1 Origin",
            json.dumps("landscape"),
            json.dumps("Bern Castle Landscape"),
            json.dumps(f"UE3 Landscape exact: {asset_id}"),
            "Opaque Back 1 1 0 0 1 1 1 50 1 1 1 1 1",
        )
        return " ".join(values)

    @staticmethod
    def landscape_placement_row(source_id: str, asset_id: str) -> str:
        return " ".join(
            (
                str(imported_id(source_id)),
                json.dumps(source_id),
                json.dumps("LV_BER_BERNCASTLE_T_LAND01"),
                json.dumps("component"),
                json.dumps(asset_id),
                "0 0 0 0 0 0 1 1 1 1 1",
            )
        )

    def create_fixture(self, root: Path) -> argparse.Namespace:
        first_directory = root / "placements-first"
        second_directory = root / "placements-second"
        runtime_root = root / "runtime"
        output_directory = root / "output"
        runtime_root.mkdir()

        source_rows = [
            self.source_placement(
                "source_base_ps",
                "asset_base_ps",
                "LV_BER_BERNCASTLE_T_PS",
                (-1.0, 1.0, 1.0),
            ),
            self.source_placement(
                "source_base_event",
                "asset_base_event",
                "LV_BER_BERNCASTLE_T_EVENT01",
            ),
        ]
        for shard_id in SL_SHARD_IDS:
            scale = (-1.0, -1.0, 1.0) if shard_id == "SL02" else (1.0, 1.0, 1.0)
            source_rows.append(
                self.source_placement(
                    f"source_{shard_id.lower()}",
                    f"asset_{shard_id.lower()}",
                    f"LV_BER_BERNCASTLE_T_{shard_id}",
                    scale,
                )
            )

        assets = [
            self.static_asset(str(row["asset"]["objectPath"]).split(".")[-1])
            for row in source_rows
        ]
        asset_manifest = root / "assets.json"
        runtime_manifest = root / "runtime.json"
        self.write_json(
            asset_manifest,
            {
                "schemaVersion": 1,
                "areaId": AREA_ID,
                "assetCount": len(assets),
                "assets": assets,
            },
        )
        runtime_rows = []
        for asset in assets:
            asset_id = str(asset["assetId"])
            model = f"{asset_id}.wmodel"
            (runtime_root / model).write_bytes(b"WMOD-fixture")
            runtime_rows.append({"assetId": asset_id, "model": model})
        self.write_json(
            runtime_manifest,
            {"schemaVersion": 1, "areaId": AREA_ID, "assets": runtime_rows},
        )
        self.write_json(
            first_directory / "shared.placements.json",
            {
                "schemaVersion": 1,
                "propertyErrors": [],
                "placements": source_rows[:7],
            },
        )
        self.write_json(
            second_directory / "shared.placements.json",
            {
                "schemaVersion": 1,
                "propertyErrors": [],
                "placements": source_rows[7:],
            },
        )

        landscape_catalog = root / "source-landscape.mapassets"
        landscape_placements = root / "source-landscape.mapplacements"
        landscape_asset_ids = ("landscape_a", "landscape_b")
        landscape_catalog_rows = [
            self.landscape_catalog_row(asset_id) for asset_id in landscape_asset_ids
        ]
        landscape_placement_rows = [
            self.landscape_placement_row(f"source_{asset_id}", asset_id)
            for asset_id in landscape_asset_ids
        ]
        landscape_catalog.write_text(
            'LOSTARK_MAP_ASSET_CATALOG 4 "LV_BER_BERNCASTLE_LANDSCAPE" 2\n'
            + "\n".join(landscape_catalog_rows)
            + "\n",
            encoding="utf-8",
        )
        landscape_placements.write_text(
            'LOSTARK_MAP_PLACEMENTS 2 "LV_BER_BERNCASTLE_LANDSCAPE" 2\n'
            + "\n".join(landscape_placement_rows)
            + "\n",
            encoding="utf-8",
        )

        level_specs = [
            "LV_BER_BERNCASTLE_T_PS=1",
            "LV_BER_BERNCASTLE_T_EVENT01=1",
            *(f"LV_BER_BERNCASTLE_T_{shard_id}=1" for shard_id in SL_SHARD_IDS),
        ]
        return argparse.Namespace(
            asset_manifest=asset_manifest,
            runtime_manifest=runtime_manifest,
            runtime_root=runtime_root,
            runtime_asset_root=None,
            overlay_manifest=None,
            render_profile_manifest=None,
            placements_dir=[first_directory, second_directory],
            landscape_catalog=landscape_catalog,
            landscape_placements=landscape_placements,
            output_dir=output_directory,
            expect_static_assets=13,
            expect_static_placements=13,
            expect_landscape_assets=2,
            expect_landscape_placements=2,
            expect_total_placements=15,
            expect_unique_assets=15,
            expect_any_negative=2,
            expect_reflected=1,
            expect_overlay_assets=None,
            expect_overlay_placements=None,
            expect_overlay_any_negative=None,
            expect_overlay_reflected=None,
            default_hidden_level=None,
            expect_default_hidden_placements=1,
            max_catalog_assets=512,
            expect_level_count=level_specs,
        )

    def attach_render_profile(
        self,
        root: Path,
        arguments: argparse.Namespace,
        asset_id: str = "asset_base_ps",
    ) -> None:
        manifest = root / "bern.renderprofiles.json"
        self.write_json(
            manifest,
            {
                "schemaVersion": 1,
                "areaId": AREA_ID,
                "profiles": [
                    {
                        "assetId": asset_id,
                        "renderMode": "Sky",
                        "cullMode": "None",
                        "colorTint": [0.12, 0.14, 0.18, 1.0],
                    }
                ],
                "visibilityOverrides": [],
            },
        )
        arguments.render_profile_manifest = manifest

    @staticmethod
    def overlay_placement(
        placement_id: int,
        source_id: str,
        asset_id: str,
        source_level: str,
        scale=(1.0, 1.0, 1.0),
    ) -> dict:
        return {
            "placementId": placement_id,
            "sourcePlacementId": source_id,
            "sourceLevel": source_level,
            "transformSource": "overlay",
            "assetId": asset_id,
            "position": [1.0, 2.0, 3.0],
            "quaternion": [0.0, 0.0, 0.0, 1.0],
            "scale": list(scale),
            "visible": True,
        }

    def attach_overlay(self, root: Path, arguments: argparse.Namespace) -> None:
        runtime_asset_root = root / "runtime-assets"
        model_path = (
            Path("Map")
            / AREA_ID
            / "foliage_extra"
            / "foliage_extra.wmodel"
        )
        model_absolute = runtime_asset_root / model_path
        model_absolute.parent.mkdir(parents=True)
        model_absolute.write_bytes(b"WMOD-overlay-fixture")
        overlay_manifest = root / "foliage.overlay.json"
        overlay_placements = [
            self.overlay_placement(
                101,
                "foliage:base:supplemental",
                "foliage_extra",
                "LV_BER_BERNCASTLE_T_EVENT01",
            ),
            self.overlay_placement(
                102,
                "foliage:sl00:reused-exact",
                "asset_sl05",
                "LV_BER_BERNCASTLE_T_SL00",
                (-1.0, 1.0, 1.0),
            ),
            self.overlay_placement(
                103,
                "foliage:sl01:supplemental",
                "foliage_extra",
                "LV_BER_BERNCASTLE_T_SL01",
                (-1.0, -1.0, 1.0),
            ),
        ]
        self.write_json(
            overlay_manifest,
            {
                "schemaVersion": 1,
                "areaId": AREA_ID,
                "kind": "foliage-native-instance-overlay",
                "summary": {
                    "overlayAssetDefinitionCount": 1,
                    "instanceCount": 3,
                    "anyNegativeScaleCount": 2,
                    "reflectedCount": 1,
                },
                "assets": [
                    {
                        "assetId": "foliage_extra",
                        "label": "Foliage Extra",
                        "modelPath": model_path.as_posix(),
                        "prototypeTag": "Prototype_Component_Model_foliage_extra",
                        "defaultScale": [1.0, 1.0, 1.0],
                        "anchor": "Origin",
                        "groupId": "foliage",
                        "groupLabel": "Bern Castle Foliage",
                        "evidence": "fixture supplemental foliage model",
                    }
                ],
                "placements": overlay_placements,
            },
        )
        arguments.runtime_asset_root = runtime_asset_root
        arguments.overlay_manifest = overlay_manifest
        arguments.expect_overlay_assets = 1
        arguments.expect_overlay_placements = 3
        arguments.expect_overlay_any_negative = 2
        arguments.expect_overlay_reflected = 1
        arguments.expect_total_placements = 18
        arguments.expect_unique_assets = 16
        arguments.expect_any_negative = 4
        arguments.expect_reflected = 2
        arguments.expect_default_hidden_placements = 2

    def test_builds_all_shards_with_pruned_catalogs_and_stable_mapset(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            arguments = self.create_fixture(root)
            source_landscape_catalog_rows = arguments.landscape_catalog.read_text(
                encoding="utf-8"
            ).splitlines()[1:]
            source_landscape_placement_rows = arguments.landscape_placements.read_text(
                encoding="utf-8"
            ).splitlines()[1:]

            receipt = build_shards(arguments)

            self.assertEqual(receipt["shardCount"], 13)
            self.assertEqual(receipt["placementCount"], 15)
            self.assertEqual(receipt["uniqueAssetCount"], 15)
            self.assertEqual(receipt["anyNegativeScaleCount"], 2)
            self.assertEqual(receipt["reflectedScaleCount"], 1)
            self.assertEqual(receipt["defaultHiddenPlacementCount"], 1)
            self.assertEqual(
                receipt["defaultHiddenLevels"],
                [
                    "LV_BER_BERNCASTLE_T_EVENT01",
                    "LV_BER_BERNCASTLE_T_SCENE03E",
                ],
            )
            self.assertEqual(
                {row["key"] for row in receipt["inputs"]["placements"]},
                {"0/shared.placements.json", "1/shared.placements.json"},
            )

            for shard_id in SHARD_IDS:
                catalog_path = arguments.output_dir / shard_file_name(shard_id, "mapassets")
                placements_path = (
                    arguments.output_dir / shard_file_name(shard_id, "mapplacements")
                )
                catalog = parse_catalog(catalog_path)
                placements = parse_placements(placements_path, catalog["assetIds"])
                self.assertEqual(catalog["areaId"], AREA_ID)
                self.assertEqual(placements["areaId"], AREA_ID)
                if shard_id == "BASE":
                    self.assertEqual(len(catalog["assetIds"]), 2)
                    self.assertEqual(len(placements["rows"]), 2)
                    self.assertEqual(placements["hiddenPlacementCount"], 1)
                elif shard_id == "LANDSCAPE":
                    self.assertEqual(len(catalog["assetIds"]), 2)
                    self.assertEqual(len(placements["rows"]), 2)
                else:
                    self.assertEqual(len(catalog["assetIds"]), 1)
                    self.assertEqual(len(placements["rows"]), 1)

            output_landscape_catalog_rows = (
                arguments.output_dir
                / shard_file_name("LANDSCAPE", "mapassets")
            ).read_text(encoding="utf-8").splitlines()[1:]
            output_landscape_placement_rows = (
                arguments.output_dir
                / shard_file_name("LANDSCAPE", "mapplacements")
            ).read_text(encoding="utf-8").splitlines()[1:]
            self.assertEqual(output_landscape_catalog_rows, source_landscape_catalog_rows)
            self.assertEqual(output_landscape_placement_rows, source_landscape_placement_rows)

            mapset = (arguments.output_dir / f"{AREA_ID}.mapset").read_text(
                encoding="utf-8"
            ).splitlines()
            self.assertEqual(mapset[0], f'LOSTARK_MAP_SHARD_SET 1 "{AREA_ID}" 13')
            self.assertEqual(len(mapset), 14)
            self.assertEqual(
                [json.loads(line.split(" ", 1)[0]) for line in mapset[1:]],
                list(SHARD_IDS),
            )
            self.assertTrue(
                (arguments.output_dir / f"{AREA_ID}.shards.receipt.json").is_file()
            )
            self.assertFalse(any(arguments.output_dir.glob(f".{AREA_ID}.stage.*")))

    def test_stable_asset_render_profile_survives_shard_regeneration(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            arguments = self.create_fixture(root)
            self.attach_render_profile(root, arguments)

            receipt = build_shards(arguments)

            base_catalog = parse_catalog(
                arguments.output_dir / shard_file_name("BASE", "mapassets")
            )
            matching_rows = [
                row
                for row in base_catalog["rows"]
                if shlex.split(row, posix=True)[0] == "asset_base_ps"
            ]
            self.assertEqual(len(matching_rows), 1)
            fields = shlex.split(matching_rows[0], posix=True)
            self.assertEqual(fields[11:13], ["Sky", "None"])
            self.assertEqual(fields[21:25], ["0.12", "0.14", "0.18", "1"])
            self.assertIn("renderProfileManifest", receipt["inputs"])

    def test_unknown_render_profile_fails_without_replacing_outputs(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            arguments = self.create_fixture(root)
            self.attach_render_profile(root, arguments, "missing_asset")
            arguments.output_dir.mkdir()
            existing = arguments.output_dir / shard_file_name("BASE", "mapassets")
            existing.write_bytes(b"existing-catalog")

            with self.assertRaisesRegex(
                ShardBuildError, "render profiles reference unknown Bern assets"
            ):
                build_shards(arguments)
            self.assertEqual(existing.read_bytes(), b"existing-catalog")

    def test_global_id_collision_fails_without_replacing_existing_outputs(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            arguments = self.create_fixture(root)
            arguments.output_dir.mkdir()
            existing = arguments.output_dir / shard_file_name("BASE", "mapassets")
            existing.write_bytes(b"existing-catalog")

            landscape_lines = arguments.landscape_placements.read_text(
                encoding="utf-8"
            ).splitlines()
            landscape_lines[1] = self.landscape_placement_row(
                "source_base_ps", "landscape_a"
            )
            arguments.landscape_placements.write_text(
                "\n".join(landscape_lines) + "\n", encoding="utf-8"
            )

            with self.assertRaisesRegex(
                ShardBuildError, "global source placement ID collision"
            ):
                build_shards(arguments)
            self.assertEqual(existing.read_bytes(), b"existing-catalog")
            self.assertFalse((arguments.output_dir / f"{AREA_ID}.mapset").exists())
            self.assertFalse(any(arguments.output_dir.glob(f".{AREA_ID}.stage.*")))

    def test_overlay_is_routed_and_reused_exact_assets_remain_in_catalog(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            arguments = self.create_fixture(root)
            self.attach_overlay(root, arguments)

            receipt = build_shards(arguments)

            self.assertEqual(receipt["overlayAssetCount"], 1)
            self.assertEqual(receipt["overlayReferencedAssetCount"], 2)
            self.assertEqual(receipt["overlayPlacementCount"], 3)
            self.assertEqual(receipt["placementCount"], 18)
            self.assertEqual(receipt["uniqueAssetCount"], 16)
            self.assertEqual(receipt["anyNegativeScaleCount"], 4)
            self.assertEqual(receipt["reflectedScaleCount"], 2)
            self.assertEqual(receipt["defaultHiddenPlacementCount"], 2)
            self.assertEqual(
                receipt["overlayLevelCounts"],
                {
                    "LV_BER_BERNCASTLE_T_EVENT01": 1,
                    "LV_BER_BERNCASTLE_T_SL00": 1,
                    "LV_BER_BERNCASTLE_T_SL01": 1,
                },
            )

            base_catalog = parse_catalog(
                arguments.output_dir / shard_file_name("BASE", "mapassets")
            )
            base_placements = parse_placements(
                arguments.output_dir / shard_file_name("BASE", "mapplacements"),
                base_catalog["assetIds"],
            )
            self.assertEqual(
                base_catalog["assetIds"],
                {"asset_base_ps", "asset_base_event", "foliage_extra"},
            )
            self.assertIn("foliage:base:supplemental", base_placements["sourceIds"])
            self.assertEqual(base_placements["hiddenPlacementCount"], 2)

            sl00_catalog = parse_catalog(
                arguments.output_dir / shard_file_name("SL00", "mapassets")
            )
            sl00_placements = parse_placements(
                arguments.output_dir / shard_file_name("SL00", "mapplacements"),
                sl00_catalog["assetIds"],
            )
            self.assertEqual(sl00_catalog["assetIds"], {"asset_sl00", "asset_sl05"})
            self.assertNotIn("foliage_extra", sl00_catalog["assetIds"])
            self.assertEqual(sl00_placements["overlayPlacementCount"], 1)
            self.assertIn("foliage:sl00:reused-exact", sl00_placements["sourceIds"])

            sl01_catalog = parse_catalog(
                arguments.output_dir / shard_file_name("SL01", "mapassets")
            )
            self.assertEqual(sl01_catalog["assetIds"], {"asset_sl01", "foliage_extra"})
            sl02_catalog = parse_catalog(
                arguments.output_dir / shard_file_name("SL02", "mapassets")
            )
            self.assertEqual(sl02_catalog["assetIds"], {"asset_sl02"})

    def test_unresolved_source_fails_without_replacing_existing_outputs(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            arguments = self.create_fixture(root)
            arguments.output_dir.mkdir()
            existing = arguments.output_dir / shard_file_name("BASE", "mapassets")
            existing.write_bytes(b"existing-catalog")
            source_path = arguments.placements_dir[0] / "shared.placements.json"
            source = json.loads(source_path.read_text(encoding="utf-8"))
            source["unresolvedPlacements"] = [
                {
                    "placementId": "unresolved:1",
                    "reason": "unsupported-component-owner",
                }
            ]
            self.write_json(source_path, source)

            with self.assertRaisesRegex(
                ShardBuildError, "placement source contains unresolved owners"
            ):
                build_shards(arguments)
            self.assertEqual(existing.read_bytes(), b"existing-catalog")
            self.assertFalse((arguments.output_dir / f"{AREA_ID}.mapset").exists())

    def test_overlay_count_gate_fails_before_replacing_existing_outputs(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            arguments = self.create_fixture(root)
            self.attach_overlay(root, arguments)
            arguments.expect_overlay_placements = 4
            arguments.output_dir.mkdir()
            existing = arguments.output_dir / shard_file_name("BASE", "mapassets")
            existing.write_bytes(b"existing-catalog")

            with self.assertRaisesRegex(
                ShardBuildError, "overlay placement count mismatch: 3"
            ):
                build_shards(arguments)
            self.assertEqual(existing.read_bytes(), b"existing-catalog")
            self.assertFalse((arguments.output_dir / f"{AREA_ID}.mapset").exists())

    def test_physical_catalog_limit_is_a_precommit_gate(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            arguments = self.create_fixture(root)
            arguments.max_catalog_assets = 1
            with self.assertRaisesRegex(
                ShardBuildError, "BASE physical catalog exceeds 1: 2"
            ):
                build_shards(arguments)
            self.assertFalse((arguments.output_dir / f"{AREA_ID}.mapset").exists())

    def test_mapset_rejects_absolute_and_traversal_paths(self):
        for value in ("../child.mapassets", "folder/child.mapassets", "C:\\child.mapassets"):
            with self.subTest(value=value):
                with self.assertRaises(ShardBuildError):
                    validate_relative_filename(value)


if __name__ == "__main__":
    unittest.main()
