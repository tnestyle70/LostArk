#!/usr/bin/env python3

from __future__ import annotations

import importlib.util
import struct
import sys
import tempfile
import unittest
from pathlib import Path


SCRIPT = Path(__file__).with_name("new_character_equipment_representative_pack.py")
SPEC = importlib.util.spec_from_file_location("representative_pack", SCRIPT)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


def bone(name: str, diagonal: float = 1.0):
    matrix = tuple(
        diagonal if row == column else 0.0
        for row in range(4)
        for column in range(4)
    )
    return MODULE.BoneRecord(hash(name) & 0xFFFFFFFF, name, -1, matrix)


def model(path: str, names: list[str], weighted: set[int]):
    bones = tuple(bone(name) for name in names)
    return MODULE.ModelInfo(
        Path(path),
        3,
        0,
        (1, 2, 3),
        1,
        3,
        bones,
        bones,
        frozenset(weighted),
        tuple(),
    )


def legacy_skinned_mesh_blob(
    *,
    vertex_format_flags: int | None = None,
    blend_indices: tuple[int, int, int, int] = (0, 0, 0, 0),
    blend_weights: tuple[float, float, float, float] = (1.0, 0.0, 0.0, 0.0),
):
    flags = (
        MODULE.VF_STATIC_BASE | MODULE.VF_BONE_WEIGHT
        if vertex_format_flags is None
        else vertex_format_flags
    )
    vertex = bytearray(MODULE.STRIDE_SKINNED)
    struct.pack_into("<3f3f2f3f", vertex, 0, *(0.0,) * 11)
    struct.pack_into("<4I", vertex, 44, *blend_indices)
    struct.pack_into("<4f", vertex, 60, *blend_weights)
    indices = struct.pack("<3H", 0, 0, 0)
    matrix = tuple(
        1.0 if row == column else 0.0
        for row in range(4)
        for column in range(4)
    )
    bone_payload = MODULE.MESH_BONE.pack(
        1,
        b"b_root\0".ljust(32, b"\0"),
        -1,
        *matrix,
        0,
        b"\0" * 16,
    )
    mesh_header = MODULE.MESH_HEADER.pack(
        b"WMSH",
        1,
        1,
        flags,
        MODULE.STRIDE_SKINNED,
        1,
        3,
        2,
        0,
        b"\0" * 3,
    )
    submesh = MODULE.SUBMESH_DESC.pack(0, 1, 0, 3, 0, 0, b"mesh\0".ljust(20, b"\0"))
    payload = mesh_header + submesh + bytes(vertex) + indices + bone_payload
    return MODULE.FILE_HEADER.pack(b"WINT", 1, 0, 0, len(payload)) + payload


def minimal_selection():
    classes = sorted(MODULE.ALLOWED_CLASSES)
    sets = []
    for index, class_id in enumerate(classes):
        sets.append(
            {
                "visualSetId": f"character.test.{class_id.lower()}",
                "classId": class_id,
                "curatedVariantId": f"variant_{index}",
                "catalogStatus": "READY_ALTERNATIVE",
                "primarySlot": "HEAD",
                "coverageSlots": ["HEAD"],
                "masterBodyAssetId": f"Character/{class_id}/body.wmodel",
                "bodyCoveragePolicy": "INHERIT_BASELINE",
                "parts": [
                    {
                        "partId": "head",
                        "sourceClassId": class_id,
                        "sourcePackageName": f"PACKAGE_{index}",
                        "sourceObjectName": f"object_{index}",
                        "expectedSourceRole": "APPAREL_HEAD",
                        "existingModelAssetId": f"Character/{class_id}/head.wmodel",
                        "targetModelAssetId": f"Character/{class_id}/Equipment/variant_{index}/head.wmodel",
                        "partRole": "HEAD",
                        "attachmentMode": "SKINNED",
                        "modelKind": "ANIM_SKINNED",
                    }
                ],
            }
        )
    return {
        "schema": MODULE.SELECTION_SCHEMA,
        "formatVersion": 1,
        "sourceAssociationPolicy": MODULE.SOURCE_ASSOCIATION_POLICY,
        "sourceInventory": "out/inventory.json",
        "pendingNormalizationCandidates": [
            {
                "candidateId": f"candidate.{class_id.lower()}.shoulder",
                "classId": class_id,
                "categoryId": "APPAREL_SHOULDER",
                "sourcePackageName": f"PACKAGE_{index}",
                "sourceObjectName": f"shoulder_{index}",
                "expectedSourceRole": "APPAREL_SHOULDER",
                "runtimeReadiness": MODULE.PENDING_NORMALIZATION_STATE,
                "rawJointCount": 10,
                "bodyPaletteBoneCount": 12,
                "additionalBlockers": [],
            }
            for index, class_id in enumerate(classes)
        ],
        "sets": sets,
    }


class RelativePathTests(unittest.TestCase):
    def test_resource_id_accepts_character_relative_wmodel(self):
        self.assertEqual(
            MODULE.normalize_resource_asset_id(
                "Character/LanceMaster/Equipment/set/head.wmodel", "asset"
            ),
            "Character/LanceMaster/Equipment/set/head.wmodel",
        )

    def test_resource_id_rejects_escape_and_absolute_paths(self):
        for value in (
            "../Character/head.wmodel",
            "Character/../head.wmodel",
            "C:/Character/head.wmodel",
            "/Character/head.wmodel",
        ):
            with self.subTest(value=value):
                with self.assertRaises(MODULE.PackError):
                    MODULE.normalize_resource_asset_id(value, "asset")

    def test_resolved_under_rejects_symlink_escape(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary) / "root"
            outside = Path(temporary) / "outside"
            root.mkdir()
            outside.mkdir()
            link = root / "link"
            try:
                link.symlink_to(outside, target_is_directory=True)
            except OSError:
                self.skipTest("symlink creation is unavailable")
            with self.assertRaises(MODULE.PackError):
                MODULE.resolved_under(root, "link/file.bin", "fixture")

    def test_pack_output_must_be_strictly_below_admission_root(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary) / "Admission"
            root.mkdir()
            expected = (root / "representative-test").resolve()
            self.assertEqual(
                MODULE.validate_pack_output(root, expected), expected
            )
            for candidate in (
                root,
                root.parent / "sibling",
                root / "outer" / "nested",
                root / "unsafe name",
            ):
                with self.subTest(candidate=candidate):
                    with self.assertRaises(MODULE.PackError):
                        MODULE.validate_pack_output(root, candidate)


class SelectionTests(unittest.TestCase):
    def test_valid_selection_covers_all_six_classes(self):
        selection = minimal_selection()
        self.assertIs(MODULE.validate_selection_document(selection), selection)

    def test_build_provenance_must_not_be_claimed(self):
        selection = minimal_selection()
        selection["sourceAssociationPolicy"] = "EXACT_BUILD_PROVENANCE"
        with self.assertRaisesRegex(MODULE.PackError, "sourceAssociationPolicy"):
            MODULE.validate_selection_document(selection)

    def test_pending_candidates_must_cover_six_classes(self):
        selection = minimal_selection()
        selection["pendingNormalizationCandidates"].pop()
        with self.assertRaisesRegex(MODULE.PackError, "one candidate per class"):
            MODULE.validate_selection_document(selection)

    def test_duplicate_visual_set_is_rejected(self):
        selection = minimal_selection()
        selection["sets"][1]["visualSetId"] = selection["sets"][0]["visualSetId"]
        with self.assertRaisesRegex(MODULE.PackError, "duplicate visualSetId"):
            MODULE.validate_selection_document(selection)

    def test_duplicate_target_model_is_rejected(self):
        selection = minimal_selection()
        selection["sets"][1]["parts"][0]["targetModelAssetId"] = (
            selection["sets"][0]["parts"][0]["targetModelAssetId"]
        )
        with self.assertRaisesRegex(MODULE.PackError, "duplicate targetModelAssetId"):
            MODULE.validate_selection_document(selection)

    def test_case_only_duplicate_target_model_is_rejected(self):
        selection = minimal_selection()
        target = selection["sets"][0]["parts"][0]["targetModelAssetId"]
        selection["sets"][1]["parts"][0]["targetModelAssetId"] = target.replace(
            "/head.wmodel", "/HEAD.wmodel"
        )
        with self.assertRaisesRegex(MODULE.PackError, "duplicate targetModelAssetId"):
            MODULE.validate_selection_document(selection)

    def test_socketed_part_requires_socket(self):
        selection = minimal_selection()
        part = selection["sets"][0]["parts"][0]
        part["attachmentMode"] = "SOCKETED"
        part["modelKind"] = "NONANIM_SOCKETED"
        with self.assertRaisesRegex(MODULE.PackError, "socketBone"):
            MODULE.validate_selection_document(selection)


class InventoryResolutionTests(unittest.TestCase):
    @staticmethod
    def entry(source_path: str, part_role: str = "APPAREL_HEAD"):
        return {
            "classId": "ARTIST",
            "visualSetId": "character.artist.pc_sdm_00",
            "packageName": "PC_SDM_00",
            "objectName": "pc_sdm_00_helmet1_sk",
            "partRole": part_role,
            "extractionState": "EXTRACTED",
            "sourcePath": source_path,
        }

    def test_duplicate_root_and_mesh_rows_choose_single_mesh_row(self):
        key = ("ARTIST", "PC_SDM_00", "pc_sdm_00_helmet1_sk")
        root = self.entry("out/Raw/Artist/PC_SDM_00/pc_sdm_00_helmet1_sk.gltf")
        mesh = self.entry("out/Raw/Artist/PC_SDM_00/mesh/pc_sdm_00_helmet1_sk.gltf")
        self.assertIs(MODULE.resolve_inventory_entry({key: [root, mesh]}, key), mesh)

    def test_multiple_mesh_rows_are_rejected_as_ambiguous(self):
        key = ("ARTIST", "PC_SDM_00", "pc_sdm_00_helmet1_sk")
        first = self.entry("out/Raw/A/mesh/pc_sdm_00_helmet1_sk.gltf")
        second = self.entry("out/Raw/B/mesh/pc_sdm_00_helmet1_sk.gltf")
        with self.assertRaisesRegex(MODULE.PackError, "ambiguous"):
            MODULE.resolve_inventory_entry({key: [first, second]}, key)


class LegacyMeshContractTests(unittest.TestCase):
    def test_engine_compatible_legacy_skinned_header_is_accepted(self):
        submeshes, vertices, bones, weighted = MODULE._parse_mesh(
            legacy_skinned_mesh_blob(), "fixture"
        )
        self.assertEqual((submeshes, vertices, len(bones), weighted), (1, 1, 1, {0}))

    def test_bone_count_without_bone_weight_flag_is_rejected(self):
        with self.assertRaisesRegex(MODULE.PackError, "legacy flags"):
            MODULE._parse_mesh(
                legacy_skinned_mesh_blob(vertex_format_flags=MODULE.VF_STATIC_BASE),
                "fixture",
            )

    def test_zero_weight_out_of_range_blend_index_is_rejected(self):
        with self.assertRaisesRegex(MODULE.PackError, "blend indices"):
            MODULE._parse_mesh(
                legacy_skinned_mesh_blob(
                    blend_indices=(0, 7, 0, 0),
                    blend_weights=(1.0, 0.0, 0.0, 0.0),
                ),
                "fixture",
            )


class WeightedPaletteTests(unittest.TestCase):
    def test_unweighted_synthetic_tail_name_may_differ(self):
        body = model("body.wmodel", ["RootNode", "b_root", "BodyMesh"], {1})
        part = model("part.wmodel", ["RootNode", "b_root", "PartMesh"], {1})
        result = MODULE.compare_weighted_palette(part, body)
        self.assertEqual(result["weightedBoneCount"], 1)
        self.assertEqual(result["fullPaletteNameMismatchCount"], 1)
        self.assertEqual(result["maxWeightedInverseBindDelta"], 0.0)

    def test_weighted_name_mismatch_is_rejected(self):
        body = model("body.wmodel", ["RootNode", "b_root", "BodyMesh"], {1})
        part = model("part.wmodel", ["RootNode", "wrong_bone", "PartMesh"], {1})
        with self.assertRaisesRegex(MODULE.PackError, "weighted palette bone"):
            MODULE.compare_weighted_palette(part, body)

    def test_weighted_inverse_bind_mismatch_is_rejected(self):
        body = model("body.wmodel", ["RootNode", "b_root", "BodyMesh"], {1})
        part = model("part.wmodel", ["RootNode", "b_root", "PartMesh"], {1})
        altered = list(part.mesh_bones)
        different = MODULE.BoneRecord(
            altered[1].name_hash,
            altered[1].name,
            altered[1].parent_index,
            tuple(2.0 if index == 0 else value for index, value in enumerate(altered[1].matrix)),
        )
        altered[1] = different
        part = MODULE.ModelInfo(
            part.path,
            part.section_count,
            part.animation_count,
            part.section_types,
            part.submesh_count,
            part.vertex_count,
            tuple(altered),
            tuple(altered),
            part.positive_weight_indices,
            part.texture_paths,
        )
        with self.assertRaisesRegex(MODULE.PackError, "inverse bind differs"):
            MODULE.compare_weighted_palette(part, body)


if __name__ == "__main__":
    unittest.main()
