import hashlib
import json
import os
from pathlib import Path
import tempfile
import unittest

from publish_character_equipment import (
    ADMISSION_FILE_NAME,
    EXPECTED_RESOURCE_FOLDERS,
    PublishError,
    build_publish_plan,
    publish_plan,
)


CLASS_FOLDERS = [
    "LanceMaster",
    "GunSlinger",
    "Slayer",
    "Artist",
    "DimensionMaster",
    "Warlord",
]


def sha256_bytes(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest().upper()


class CharacterEquipmentPublisherTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.admission_root = self.root / "Admission"
        self.resource_root = self.root / "Resources"
        self.runtime_root = self.admission_root / "Runtime"
        self.source_root = self.admission_root / "Source"
        self.runtime_root.mkdir(parents=True)
        self.source_root.mkdir(parents=True)
        for folder in EXPECTED_RESOURCE_FOLDERS:
            (self.resource_root / folder).mkdir(parents=True, exist_ok=True)
        for class_folder in CLASS_FOLDERS:
            (self.resource_root / "Character" / class_folder).mkdir(parents=True)
        (self.source_root / "raw.gltf").write_bytes(b"raw-source-must-not-publish")
        self.receipt = self._make_receipt()
        self._write_receipt()

    def tearDown(self):
        self.temporary.cleanup()

    def _make_receipt(self):
        closure = []
        model_asset_ids = []
        for index in range(73):
            class_folder = CLASS_FOLDERS[index % len(CLASS_FOLDERS)]
            extension = "wmodel" if index < 17 else "tga"
            asset_id = (
                f"Character/{class_folder}/Equipment/set_{index % 12}/"
                f"asset_{index:02d}.{extension}"
            )
            payload = f"runtime-{index}".encode("ascii")
            runtime_file = self.runtime_root.joinpath(*asset_id.split("/"))
            runtime_file.parent.mkdir(parents=True, exist_ok=True)
            runtime_file.write_bytes(payload)
            closure.append(
                {
                    "kind": "WMODEL" if extension == "wmodel" else "TEXTURE",
                    "assetId": asset_id,
                    "byteSize": len(payload),
                    "sha256": sha256_bytes(payload),
                }
            )
            if index < 17:
                model_asset_ids.append(asset_id)

        parts_per_set = [2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 2, 2]
        sets = []
        model_index = 0
        for set_index, part_count in enumerate(parts_per_set):
            parts = []
            for part_index in range(part_count):
                parts.append(
                    {
                        "partId": f"part_{part_index}",
                        "targetModelAssetId": model_asset_ids[model_index],
                    }
                )
                model_index += 1
            sets.append(
                {
                    "visualSetId": f"test.set.{set_index}",
                    "classId": f"CLASS_{set_index % 6}",
                    "parts": parts,
                }
            )
        return {
            "schema": "lostark.character-equipment-runtime-admission",
            "formatVersion": 1,
            "counts": {
                "classCount": 6,
                "visualSetCount": 12,
                "partCount": 17,
                "runtimeClosureFileCount": 73,
            },
            "sets": sets,
            "runtimeClosure": closure,
        }

    def _write_receipt(self):
        (self.admission_root / ADMISSION_FILE_NAME).write_text(
            json.dumps(self.receipt, indent=2),
            encoding="utf-8",
        )

    def test_validate_accepts_hash_verified_73_file_closure_without_mutation(self):
        plan = build_publish_plan(self.admission_root, self.resource_root)
        self.assertEqual(73, len(plan.entries))
        self.assertEqual(6, len(plan.groups))
        self.assertEqual(6, len(plan.new_groups))
        self.assertFalse(any((self.resource_root / "Character" / item / "Equipment").exists() for item in CLASS_FOLDERS))

    def test_hash_mismatch_fails_before_publish(self):
        first = self.receipt["runtimeClosure"][0]
        self.runtime_root.joinpath(*first["assetId"].split("/")).write_bytes(b"tampered")
        with self.assertRaisesRegex(PublishError, "size mismatch|hash mismatch"):
            build_publish_plan(self.admission_root, self.resource_root)
        self.assertFalse(any((self.resource_root / "Character" / item / "Equipment").exists() for item in CLASS_FOLDERS))

    def test_unsafe_asset_path_is_rejected(self):
        self.receipt["runtimeClosure"][0]["assetId"] = "Character/LanceMaster/Equipment/../escape.wmodel"
        self._write_receipt()
        with self.assertRaisesRegex(PublishError, "Unsafe Resources-relative"):
            build_publish_plan(self.admission_root, self.resource_root)

    def test_wrong_resource_top_level_contract_is_rejected(self):
        (self.resource_root / "SourceData").mkdir()
        with self.assertRaisesRegex(PublishError, "exactly the seven"):
            build_publish_plan(self.admission_root, self.resource_root)

    def test_target_collision_fails_before_any_new_directory(self):
        first = self.receipt["runtimeClosure"][0]
        target = self.resource_root.joinpath(*first["assetId"].split("/"))
        target.parent.mkdir(parents=True)
        target.write_bytes(b"different")
        with self.assertRaisesRegex(PublishError, "target collision"):
            build_publish_plan(self.admission_root, self.resource_root)
        existing_equipment = [
            item
            for item in CLASS_FOLDERS
            if (self.resource_root / "Character" / item / "Equipment").exists()
        ]
        self.assertEqual(["LanceMaster"], existing_equipment)

    def test_publish_promotes_equipment_only_and_is_idempotent(self):
        plan = build_publish_plan(self.admission_root, self.resource_root)
        self.assertEqual("PUBLISHED", publish_plan(plan))
        for entry in plan.entries:
            self.assertTrue(entry.target_path.is_file())
            self.assertEqual(entry.source_path.read_bytes(), entry.target_path.read_bytes())
        self.assertFalse((self.resource_root / "Source").exists())
        self.assertFalse((self.resource_root / "raw.gltf").exists())

        second_plan = build_publish_plan(self.admission_root, self.resource_root)
        self.assertEqual(0, len(second_plan.new_groups))
        self.assertEqual(6, len(second_plan.existing_groups))
        self.assertEqual("PUBLISHED_NO_CHANGE", publish_plan(second_plan))

    def test_intermediate_promotion_failure_rolls_back_new_directories(self):
        plan = build_publish_plan(self.admission_root, self.resource_root)
        calls = 0

        def fail_second_promotion(source: Path, target: Path):
            nonlocal calls
            calls += 1
            if calls == 2:
                raise OSError("injected promotion failure")
            os.replace(source, target)

        with self.assertRaisesRegex(PublishError, "rolled back"):
            publish_plan(plan, promote_directory=fail_second_promotion)
        self.assertEqual(2, calls)
        self.assertFalse(any((self.resource_root / "Character" / item / "Equipment").exists() for item in CLASS_FOLDERS))
        self.assertFalse(
            list(self.resource_root.parent.glob(".Resources.character-equipment-stage-*"))
        )


if __name__ == "__main__":
    unittest.main()
