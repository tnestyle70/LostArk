import argparse
import struct
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from merge_maptool_landscape import (
    MapDocumentMergeError,
    imported_id,
    merge_landscape,
)


class MapToolLandscapeMergeTests(unittest.TestCase):
    @staticmethod
    def catalog_row(asset_id: str, *, label: str | None = None) -> str:
        name = asset_id if label is None else label
        return (
            f'"{asset_id}" "{name}" "Map/TEST/Landscape/{asset_id}/{asset_id}.wmodel" '
            f'"Prototype_Component_Model_{asset_id}" 1 1 1 Origin "landscape" '
            f'"Test Landscape" "exact" Opaque Back 1 1 0 0 1 1 1 50 1 1 1 1 1'
        )

    @staticmethod
    def placement_row(source_id: str, asset_id: str, transform_source: str = "component") -> str:
        return (
            f'{imported_id(source_id)} "{source_id}" "TEST_LAND" '
            f'"{transform_source}" "{asset_id}" 0 0 0 0 0 0 1 1 1 1 1'
        )

    @staticmethod
    def authored_placement_row(
        runtime_id: int,
        source_id: str,
        asset_id: str,
        transform_source: str,
    ) -> str:
        return (
            f'{runtime_id} "{source_id}" "EDITOR" '
            f'"{transform_source}" "{asset_id}" 0 0 0 0 0 0 1 1 1 1 1'
        )

    @staticmethod
    def write_document(path: Path, magic: str, version: int, area_id: str, rows) -> None:
        path.write_text(
            f'{magic} {version} "{area_id}" {len(rows)}\n'
            + "\n".join(rows)
            + "\n",
            encoding="utf-8",
        )

    @staticmethod
    def write_wmodel(runtime_root: Path, asset_id: str) -> None:
        path = runtime_root / "Map" / "TEST" / "Landscape" / asset_id / f"{asset_id}.wmodel"
        path.parent.mkdir(parents=True, exist_ok=True)
        mesh_content = struct.pack(
            "<4sIIIIIIIB3x",
            b"WMSH",
            1,
            0,
            0,
            48,
            0,
            0,
            4,
            0,
        ) + struct.pack("<IIIIIQ20s", 0, 0, 0, 0, 0, 0, b"")
        mesh_section = struct.pack(
            "<4sHHII", b"WINT", 1, 0, 0, len(mesh_content)
        ) + mesh_content
        material_content = struct.pack("<4sI", b"WMAT", 0)
        material_section = struct.pack(
            "<4sHHII", b"WINT", 1, 0, 0, len(material_content)
        ) + material_content

        section_table_end = 32 + 64 * 2
        mesh_offset = section_table_end
        material_offset = mesh_offset + len(mesh_section)
        model_content = (
            struct.pack("<4sIII4I", b"WMOD", 2, 0, 0, 0, 0, 0, 0)
            + struct.pack(
                "<IIQQ40s", 1, 0, mesh_offset, len(mesh_section), b"mesh"
            )
            + struct.pack(
                "<IIQQ40s",
                2,
                0,
                material_offset,
                len(material_section),
                b"material",
            )
            + mesh_section
            + material_section
        )
        path.write_bytes(
            struct.pack("<4sHHII", b"WINT", 1, 0, 0, len(model_content))
            + model_content
        )

    def fixture(self, root: Path) -> argparse.Namespace:
        base_catalog = root / "base.mapassets"
        base_placements = root / "base.mapplacements"
        landscape_catalog = root / "land.mapassets"
        landscape_placements = root / "land.mapplacements"
        runtime_root = root / "runtime"
        self.write_document(
            base_catalog,
            "LOSTARK_MAP_ASSET_CATALOG",
            4,
            "TEST",
            [self.catalog_row("BASE")],
        )
        self.write_document(
            base_placements,
            "LOSTARK_MAP_PLACEMENTS",
            2,
            "TEST",
            [self.placement_row("base:1", "BASE")],
        )
        self.write_document(
            landscape_catalog,
            "LOSTARK_MAP_ASSET_CATALOG",
            4,
            "TEST_LANDSCAPE",
            [self.catalog_row("LAND")],
        )
        self.write_document(
            landscape_placements,
            "LOSTARK_MAP_PLACEMENTS",
            2,
            "TEST_LANDSCAPE",
            [self.placement_row("land:1", "LAND")],
        )
        self.write_wmodel(runtime_root, "LAND")
        return argparse.Namespace(
            area_id="TEST",
            base_catalog=base_catalog,
            base_placements=base_placements,
            landscape_catalog=landscape_catalog,
            landscape_placements=landscape_placements,
            runtime_root=runtime_root,
            catalog_output=base_catalog,
            placement_output=base_placements,
            receipt_output=root / "receipt.json",
            expect_base_assets=None,
            expect_base_placements=None,
            expect_landscape_assets=1,
            expect_landscape_placements=1,
            expect_output_assets=2,
            expect_output_placements=2,
            check_only=False,
        )

    def test_merge_is_validated_and_idempotent(self):
        with tempfile.TemporaryDirectory() as temporary:
            args = self.fixture(Path(temporary))
            first = merge_landscape(args)
            self.assertEqual(first["addedAssetCount"], 1)
            self.assertEqual(first["addedPlacementCount"], 1)
            self.assertEqual(first["outputAssetCount"], 2)
            first_catalog = args.catalog_output.read_bytes()
            first_placements = args.placement_output.read_bytes()
            first_receipt = args.receipt_output.read_bytes()

            second = merge_landscape(args)
            self.assertEqual(second["addedAssetCount"], 0)
            self.assertEqual(second["addedPlacementCount"], 0)
            self.assertEqual(args.catalog_output.read_bytes(), first_catalog)
            self.assertEqual(args.placement_output.read_bytes(), first_placements)
            self.assertEqual(args.receipt_output.read_bytes(), first_receipt)

    def test_conflicting_existing_asset_is_rejected_without_writes(self):
        with tempfile.TemporaryDirectory() as temporary:
            args = self.fixture(Path(temporary))
            original_catalog = args.base_catalog.read_bytes()
            original_placements = args.base_placements.read_bytes()
            self.write_document(
                args.landscape_catalog,
                "LOSTARK_MAP_ASSET_CATALOG",
                4,
                "TEST_LANDSCAPE",
                [self.catalog_row("BASE", label="different")],
            )
            self.write_document(
                args.landscape_placements,
                "LOSTARK_MAP_PLACEMENTS",
                2,
                "TEST_LANDSCAPE",
                [self.placement_row("land:1", "BASE")],
            )
            self.write_wmodel(args.runtime_root, "BASE")
            with self.assertRaisesRegex(MapDocumentMergeError, "conflicting asset ID"):
                merge_landscape(args)
            self.assertEqual(args.base_catalog.read_bytes(), original_catalog)
            self.assertEqual(args.base_placements.read_bytes(), original_placements)

    def test_second_output_replace_failure_restores_base_pair(self):
        with tempfile.TemporaryDirectory() as temporary:
            args = self.fixture(Path(temporary))
            original_catalog = args.base_catalog.read_bytes()
            original_placements = args.base_placements.read_bytes()
            real_replace = __import__("os").replace
            call_count = 0

            def fail_second_replace(source, destination):
                nonlocal call_count
                call_count += 1
                if call_count == 2:
                    raise OSError("injected placement replace failure")
                return real_replace(source, destination)

            with patch(
                "merge_maptool_landscape.os.replace",
                side_effect=fail_second_replace,
            ):
                with self.assertRaisesRegex(OSError, "injected placement replace failure"):
                    merge_landscape(args)
            self.assertEqual(args.base_catalog.read_bytes(), original_catalog)
            self.assertEqual(args.base_placements.read_bytes(), original_placements)

    def test_non_component_landscape_transform_is_rejected(self):
        with tempfile.TemporaryDirectory() as temporary:
            args = self.fixture(Path(temporary))
            self.write_document(
                args.landscape_placements,
                "LOSTARK_MAP_PLACEMENTS",
                2,
                "TEST_LANDSCAPE",
                [self.placement_row("land:1", "LAND", "actor")],
            )
            with self.assertRaisesRegex(MapDocumentMergeError, "transformSource=component"):
                merge_landscape(args)

    def test_current_authoring_id_domains_are_accepted_in_base_document(self):
        with tempfile.TemporaryDirectory() as temporary:
            args = self.fixture(Path(temporary))
            rows = [self.placement_row("base:1", "BASE")]
            rows.extend(
                self.authored_placement_row(index, f"authored:{index}", "BASE", source)
                for index, source in enumerate(
                    ("editor", "legacy", "overlay"), start=1
                )
            )
            self.write_document(
                args.base_placements,
                "LOSTARK_MAP_PLACEMENTS",
                2,
                "TEST",
                rows,
            )
            args.expect_output_placements = 5

            result = merge_landscape(args)

            self.assertEqual(result["addedPlacementCount"], 1)
            self.assertEqual(result["outputPlacementCount"], 5)

    def test_unknown_base_transform_source_is_rejected(self):
        with tempfile.TemporaryDirectory() as temporary:
            args = self.fixture(Path(temporary))
            self.write_document(
                args.base_placements,
                "LOSTARK_MAP_PLACEMENTS",
                2,
                "TEST",
                [self.placement_row("base:1", "BASE", "generated")],
            )
            with self.assertRaisesRegex(
                MapDocumentMergeError, "unsupported placement transform source"
            ):
                merge_landscape(args)

    def test_missing_runtime_model_is_rejected(self):
        with tempfile.TemporaryDirectory() as temporary:
            args = self.fixture(Path(temporary))
            model = (
                args.runtime_root
                / "Map"
                / "TEST"
                / "Landscape"
                / "LAND"
                / "LAND.wmodel"
            )
            model.unlink()
            with self.assertRaisesRegex(MapDocumentMergeError, "runtime model is missing"):
                merge_landscape(args)

    def test_truncated_runtime_model_is_rejected(self):
        with tempfile.TemporaryDirectory() as temporary:
            args = self.fixture(Path(temporary))
            model = (
                args.runtime_root
                / "Map"
                / "TEST"
                / "Landscape"
                / "LAND"
                / "LAND.wmodel"
            )
            model.write_bytes(
                b"WINT"
                + (1).to_bytes(4, "little")
                + (0).to_bytes(4, "little")
                + (100).to_bytes(4, "little")
                + b"WMOD"
            )
            with self.assertRaisesRegex(MapDocumentMergeError, "invalid Landscape WModel"):
                merge_landscape(args)

    def test_outer_size_only_fake_runtime_model_is_rejected(self):
        with tempfile.TemporaryDirectory() as temporary:
            args = self.fixture(Path(temporary))
            model = (
                args.runtime_root
                / "Map"
                / "TEST"
                / "Landscape"
                / "LAND"
                / "LAND.wmodel"
            )
            payload = b"WMOD" + b"\0" * 4
            model.write_bytes(
                struct.pack("<4sHHII", b"WINT", 1, 0, 0, len(payload))
                + payload
            )
            with self.assertRaisesRegex(MapDocumentMergeError, "invalid Landscape WModel"):
                merge_landscape(args)

    def test_conflicting_existing_receipt_is_rejected_before_writes(self):
        with tempfile.TemporaryDirectory() as temporary:
            args = self.fixture(Path(temporary))
            args.receipt_output.write_text("{}\n", encoding="utf-8")
            original_catalog = args.base_catalog.read_bytes()
            original_placements = args.base_placements.read_bytes()
            with self.assertRaisesRegex(MapDocumentMergeError, "different map outputs"):
                merge_landscape(args)
            self.assertEqual(args.base_catalog.read_bytes(), original_catalog)
            self.assertEqual(args.base_placements.read_bytes(), original_placements)

    def test_check_only_rejects_conflicting_existing_receipt(self):
        with tempfile.TemporaryDirectory() as temporary:
            args = self.fixture(Path(temporary))
            args.receipt_output.write_text("{}\n", encoding="utf-8")
            args.check_only = True
            with self.assertRaisesRegex(MapDocumentMergeError, "different map outputs"):
                merge_landscape(args)

    def test_output_and_receipt_path_aliases_are_rejected(self):
        with tempfile.TemporaryDirectory() as temporary:
            args = self.fixture(Path(temporary))
            args.placement_output = args.catalog_output
            with self.assertRaisesRegex(MapDocumentMergeError, "outputs must be distinct"):
                merge_landscape(args)

            args = self.fixture(Path(temporary))
            args.receipt_output = args.catalog_output
            with self.assertRaisesRegex(MapDocumentMergeError, "receipt output"):
                merge_landscape(args)

    def test_unrelated_landscape_area_is_rejected(self):
        with tempfile.TemporaryDirectory() as temporary:
            args = self.fixture(Path(temporary))
            self.write_document(
                args.landscape_catalog,
                "LOSTARK_MAP_ASSET_CATALOG",
                4,
                "OTHER",
                [self.catalog_row("LAND")],
            )
            self.write_document(
                args.landscape_placements,
                "LOSTARK_MAP_PLACEMENTS",
                2,
                "OTHER",
                [self.placement_row("land:1", "LAND")],
            )
            with self.assertRaisesRegex(MapDocumentMergeError, "unexpected Landscape areaId"):
                merge_landscape(args)


if __name__ == "__main__":
    unittest.main()
