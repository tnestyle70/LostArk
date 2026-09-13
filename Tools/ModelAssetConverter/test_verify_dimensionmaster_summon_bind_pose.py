#!/usr/bin/env python3

from __future__ import annotations

import json
from pathlib import Path
import struct
import tempfile
import unittest
from unittest.mock import Mock, patch

import verify_dimensionmaster_summon_bind_pose as verifier


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
RUNTIME_WMODEL = (
    REPOSITORY_ROOT
    / "Client/Bin/Resources/Character/DimensionMaster/DimensionMaster_DimensionSummon.wmodel"
)
RECEIPT = (
    REPOSITORY_ROOT
    / "Data/Effects/Imported/DimensionMaster/DimensionMaster.summon-bind-pose-repair.receipt.json"
)


class WModelSelectiveReadTest(unittest.TestCase):
    """Small binary fixtures exercise skipped payload admission without Resources."""

    def setUp(self) -> None:
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.path = Path(self.directory.name) / "two-clips.wmodel"
        identity = [float(i % 5 == 0) for i in range(16)]

        def nested(payload: bytes) -> bytes:
            return verifier.FILE_HEADER.pack(b"WINT", 1, 0, 0, len(payload)) + payload

        vertex = bytearray(76)
        struct.pack_into("<3f", vertex, 0, 1, 2, 3)
        struct.pack_into("<4f", vertex, 60, 1, 0, 0, 0)
        mesh = verifier.MESH_HEADER.pack(b"WMSH", 1, 1, 0, 76, 1, 1, 2, 0, bytes(3))
        mesh += verifier.SUBMESH_DESC.pack(0, 1, 0, 1, 0, 0, bytes(20))
        mesh += vertex + struct.pack("<H", 0)
        mesh += verifier.MESH_BONE.pack(1, b"b_root", -1, *identity, 0, bytes(16))
        skeleton = verifier.SKELETON_HEADER.pack(b"WSKL", 1, 0, *([0] * 5))
        skeleton += verifier.SKELETON_BONE.pack(1, b"b_root", -1, *identity, 0, 0, *([0] * 27))

        def animation(distance: float) -> bytes:
            header = verifier.ANIMATION_HEADER.pack(b"WANM", 1, 1., 1., 0, 0, 0, bytes(7))
            channel = verifier.ANIMATION_CHANNEL.pack(1, 2, 0, 2, 32, 2, 72, 0, 0)
            keys = verifier.VECTOR_KEY.pack(0, 0, 0, 0) + verifier.VECTOR_KEY.pack(1, distance, 0, 0)
            keys += verifier.QUATERNION_KEY.pack(0, 0, 0, 0, 1) + verifier.QUATERNION_KEY.pack(1, 0, 0, 0, 1)
            keys += verifier.VECTOR_KEY.pack(0, 1, 1, 1) + verifier.VECTOR_KEY.pack(1, 1, 1, 1)
            return nested(header + channel + keys + bytes(8))

        payloads = [(1, 0, "mesh", nested(mesh)), (3, 0, "skeleton", nested(skeleton)),
                    (4, 0, "first", animation(1)), (4, 1, "second", animation(2))]
        model = verifier.MODEL_HEADER.pack(b"WMOD", len(payloads), 2, 0, *([0] * 4))
        offset = len(model) + len(payloads) * verifier.SECTION_DESC.size
        self.section_offsets = {}
        descriptors = []
        for type_id, index, name, payload in payloads:
            descriptors.append(verifier.SECTION_DESC.pack(type_id, index, offset, len(payload), name.encode()))
            self.section_offsets[name] = verifier.FILE_HEADER.size + offset
            offset += len(payload)
        self.path.write_bytes(nested(model + b"".join(descriptors) + b"".join(row[3] for row in payloads)))

    def read_without_payloads(self):
        return verifier.read_wmodel(self.path, include_geometry=False, animation_names=())

    def write_u32(self, offset: int, value: int) -> None:
        data = bytearray(self.path.read_bytes())
        struct.pack_into("<I", data, offset, value)
        self.path.write_bytes(data)

    def test_default_and_selected_clip_keep_exact_decoded_values(self) -> None:
        full = verifier.read_wmodel(self.path)
        self.assertEqual(1, len(full.vertices))
        self.assertEqual(2, len(full.animations))
        self.assertTrue(all(row.channels is not None for row in full.animations))
        selected = verifier.read_wmodel(self.path, include_geometry=False, animation_names={"second"})
        self.assertIsNone(selected.vertices)
        self.assertIsNone(selected.animations[0].channels)
        self.assertEqual(full.animations[1], selected.animations[1])
        self.assertEqual(full.skeleton_bones, selected.skeleton_bones)
        self.assertEqual(full.mesh_bones, selected.mesh_bones)
        self.assertEqual(full.submeshes, selected.submeshes)

    def test_metadata_omits_vertex_and_key_construction(self) -> None:
        full = verifier.read_wmodel(self.path)
        vector_keys = Mock(size=verifier.VECTOR_KEY.size)
        vector_keys.unpack_from.side_effect = AssertionError("vector keys decoded")
        rotation_keys = Mock(size=verifier.QUATERNION_KEY.size)
        rotation_keys.unpack_from.side_effect = AssertionError("rotation keys decoded")
        with patch.object(verifier, "Vertex", side_effect=AssertionError("vertex decoded")), \
                patch.object(verifier, "AnimationChannel", side_effect=AssertionError("channel decoded")), \
                patch.object(verifier, "VECTOR_KEY", vector_keys), \
                patch.object(verifier, "QUATERNION_KEY", rotation_keys):
            metadata = self.read_without_payloads()
        self.assertEqual([(a.name, a.duration_ticks, a.ticks_per_second) for a in full.animations],
                         [(a.name, a.duration_ticks, a.ticks_per_second) for a in metadata.animations])
        self.assertTrue(all(a.channels is None for a in metadata.animations))
        with self.assertRaisesRegex(ValueError, "decoded vertices"):
            verifier.sample_animation(metadata, metadata.animations[0], 0)
        with self.assertRaisesRegex(ValueError, "decoded channels"):
            verifier.verify_animation_motion(metadata, metadata.animations[0])

    def test_requested_clip_must_exist(self) -> None:
        with self.assertRaisesRegex(ValueError, "requested animation is missing"):
            verifier.read_wmodel(self.path, include_geometry=False, animation_names={"missing"})

    def test_skipped_clip_key_span_is_still_validated(self) -> None:
        channel = self.section_offsets["second"] + verifier.FILE_HEADER.size + verifier.ANIMATION_HEADER.size
        self.write_u32(channel + 12, 0xFFFFFFFF)
        with self.assertRaisesRegex(ValueError, "vector key span"):
            verifier.read_wmodel(self.path, include_geometry=False, animation_names={"first"})

    def test_skipped_clip_bone_reference_is_still_validated(self) -> None:
        channel = self.section_offsets["second"] + verifier.FILE_HEADER.size + verifier.ANIMATION_HEADER.size
        self.write_u32(channel, 99)
        with self.assertRaisesRegex(ValueError, "unknown bone"):
            self.read_without_payloads()

    def test_skipped_clip_duplicate_bone_is_still_validated(self) -> None:
        data = bytearray(self.path.read_bytes())
        nested = self.section_offsets["second"]
        header = nested + verifier.FILE_HEADER.size
        channel = header + verifier.ANIMATION_HEADER.size
        row = data[channel:channel + verifier.ANIMATION_CHANNEL.size]
        data[channel:channel] = row
        descriptor = verifier.FILE_HEADER.size + verifier.MODEL_HEADER.size + 3 * verifier.SECTION_DESC.size
        struct.pack_into("<I", data, 12, len(data) - verifier.FILE_HEADER.size)
        struct.pack_into("<Q", data, descriptor + 16, len(data) - nested)
        struct.pack_into("<I", data, nested + 12, len(data) - header)
        struct.pack_into("<I", data, header + 4, 2)
        self.path.write_bytes(data)
        with self.assertRaisesRegex(ValueError, "duplicates a bone"):
            self.read_without_payloads()

    def test_skipped_geometry_span_is_still_validated(self) -> None:
        submesh = self.section_offsets["mesh"] + verifier.FILE_HEADER.size + verifier.MESH_HEADER.size
        self.write_u32(submesh, 76)
        with self.assertRaisesRegex(ValueError, "submesh vertex span"):
            self.read_without_payloads()

    def test_skeleton_cannot_read_into_the_next_section(self) -> None:
        skeleton = self.section_offsets["skeleton"] + verifier.FILE_HEADER.size
        self.write_u32(skeleton + 4, 2)
        with self.assertRaisesRegex(ValueError, "WSKL bone table is truncated"):
            self.read_without_payloads()

    def test_metadata_requires_the_skeleton_section(self) -> None:
        descriptor = verifier.FILE_HEADER.size + verifier.MODEL_HEADER.size + verifier.SECTION_DESC.size
        self.write_u32(descriptor, 2)
        with self.assertRaisesRegex(ValueError, "one mesh and one skeleton"):
            self.read_without_payloads()

    def test_skipped_section_cannot_escape_the_container(self) -> None:
        descriptor = verifier.FILE_HEADER.size + verifier.MODEL_HEADER.size + 3 * verifier.SECTION_DESC.size
        self.write_u32(descriptor + 16, 0xFFFFFFFF)
        with self.assertRaisesRegex(ValueError, "section 3 is out of range"):
            self.read_without_payloads()

    def test_duplicate_sections_and_clip_names_are_rejected(self) -> None:
        data = self.path.read_bytes()
        descriptor = verifier.FILE_HEADER.size + verifier.MODEL_HEADER.size + 3 * verifier.SECTION_DESC.size
        self.write_u32(descriptor + 4, 0)
        with self.assertRaisesRegex(ValueError, "duplicates a section ID"):
            self.read_without_payloads()
        changed = bytearray(data)
        changed[descriptor + 24:descriptor + 64] = b"first" + bytes(35)
        self.path.write_bytes(changed)
        with self.assertRaisesRegex(ValueError, "duplicate animation name"):
            self.read_without_payloads()


class DimensionMasterSummonBindPoseTest(unittest.TestCase):
    def test_runtime_asset_matches_exact_receipt(self) -> None:
        receipt = json.loads(RECEIPT.read_text(encoding="utf-8"))
        witness = verifier.verify(RUNTIME_WMODEL)
        self.assertEqual(
            receipt["wmodelSha256"],
            witness["wmodelSha256"],
        )
        self.assertEqual(receipt["submeshCount"], witness["submeshCount"])
        self.assertEqual(
            receipt["cookedVertexCount"], witness["cookedVertexCount"]
        )
        self.assertEqual(
            receipt["sourceTopology"]["nonzeroInfluenceHistogram"][1],
            witness["sourceTopology"]["nonzeroInfluenceHistogram"][1],
        )
        self.assertAlmostEqual(
            witness["bindPose"]["maximumNormalizedIdentityError"],
            receipt["bindPose"]["maximumNormalizedIdentityError"],
            places=12,
        )

        self.assertEqual(
            [row["name"] for row in receipt["animations"]],
            [row["name"] for row in witness["animations"]],
        )
        for expected, actual in zip(
            receipt["animations"], witness["animations"], strict=True
        ):
            self.assertEqual(expected["durationTicks"], actual["durationTicks"])
            self.assertEqual(expected["ticksPerSecond"], actual["ticksPerSecond"])
            self.assertEqual(expected["channelCount"], actual["channelCount"])
            self.assertEqual(
                expected["movingSourceBones"], actual["movingSourceBones"]
            )
            for expected_sample, actual_sample in zip(
                expected["samples"], actual["samples"], strict=True
            ):
                self.assertEqual(expected_sample["timeTicks"], actual_sample["timeTicks"])
                for expected_value, actual_value in zip(
                    expected_sample["bounds"]["minimum"],
                    actual_sample["bounds"]["minimum"],
                    strict=True,
                ):
                    self.assertAlmostEqual(expected_value, actual_value, places=5)
                for expected_value, actual_value in zip(
                    expected_sample["bounds"]["maximum"],
                    actual_sample["bounds"]["maximum"],
                    strict=True,
                ):
                    self.assertAlmostEqual(expected_value, actual_value, places=5)
                self.assertAlmostEqual(
                    expected_sample["bounds"]["diagonal"],
                    actual_sample["bounds"]["diagonal"],
                    places=5,
                )

    def test_action_pose_baked_as_rest_is_rejected(self) -> None:
        model = verifier.read_wmodel(RUNTIME_WMODEL)
        clock = next(
            bone for bone in model.skeleton_bones if bone.name == "b_clock_17"
        )
        clock.transform[12] += 1.0
        with self.assertRaisesRegex(ValueError, "source bind/rest basis mismatch"):
            verifier.verify_bind_pose(model, 1e-3)

    def test_rest_mode_stationary_action_bake_is_rejected(self) -> None:
        model = verifier.read_wmodel(RUNTIME_WMODEL)
        animation = model.animations[0]
        for channel in animation.channels:
            for keys in (
                channel.position_keys,
                channel.rotation_keys,
                channel.scale_keys,
            ):
                if not keys:
                    continue
                source = keys[0]
                keys[:] = [tuple([key[0], *source[1:]]) for key in keys]
        with self.assertRaisesRegex(ValueError, "lost source clock motion"):
            verifier.verify_animation_motion(model, animation)


if __name__ == "__main__":
    unittest.main()
