#!/usr/bin/env python3

from __future__ import annotations

import importlib.util
import struct
import sys
import tempfile
import unittest
from pathlib import Path


SCRIPT = Path(__file__).with_name("normalize_character_equipment_gltf.py")
SPEC = importlib.util.spec_from_file_location("normalize_character_equipment_gltf", SCRIPT)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


IDENTITY = MODULE.IDENTITY4


def translation(x: float, y: float, z: float) -> tuple[float, ...]:
    matrix = list(IDENTITY)
    matrix[12], matrix[13], matrix[14] = x, y, z
    return tuple(matrix)


def master_skeleton(
    names: tuple[str, ...] = ("RootNode", "rig", "b_root", "b_spine", "body_mesh"),
    parents: tuple[int, ...] = (-1, 0, 1, 2, 0),
    mesh_slot: int = 4,
) -> "MODULE.MasterSkeleton":
    bones = tuple(
        MODULE.MasterBone(
            name,
            parent,
            translation(float(index), 0.0, 0.0),
            IDENTITY if index == mesh_slot else translation(-float(index), 0.0, 0.0),
        )
        for index, (name, parent) in enumerate(zip(names, parents))
    )
    return MODULE.MasterSkeleton("TestClass", Path("TestClass.wmodel"), bones, mesh_slot)


def gltf_bind_for(master: "MODULE.MasterSkeleton", name: str) -> tuple[float, ...]:
    """The glTF-space bind pose whose master-frame image is the master's."""
    index = master.index_of(name)
    assert index is not None
    return MODULE.conjugate(
        master.bones[index].inverse_bind,
        MODULE.MASTER_FROM_GLTF_INVERSE,
        MODULE.MASTER_FROM_GLTF,
    )


def primitive(joint_slots: tuple[int, ...]) -> "MODULE.SourcePrimitive":
    vertex_count = len(joint_slots)
    return MODULE.SourcePrimitive(
        0,
        tuple(range(vertex_count)) * 3,
        tuple((float(index), 1.0, 2.0) for index in range(vertex_count)),
        tuple((slot, 0, 0, 0) for slot in joint_slots),
        tuple((1.0, 0.0, 0.0, 0.0) for _ in joint_slots),
        {"NORMAL": tuple((0.0, 1.0, 0.0) for _ in joint_slots)},
        {"TEXCOORD_0": tuple((0.0, 0.0) for _ in joint_slots)},
    )


def source_part(
    master: "MODULE.MasterSkeleton",
    joint_names: tuple[str, ...],
    joint_parents: tuple[int, ...] | None = None,
    binds: dict[str, tuple[float, ...]] | None = None,
    weighted_slots: tuple[int, ...] | None = None,
    mesh_node_name: str = "part_mesh",
    duplicate_joint_names: bool = False,
) -> "MODULE.SourcePart":
    """Build the smallest source part that exercises one contract.

    Node 0 is the part's mesh node; the joints follow it in order, each parented
    to the previous joint unless ``joint_parents`` says otherwise.
    """
    names = [mesh_node_name, *joint_names]
    if duplicate_joint_names:
        names[-1] = names[-2]
    parents = [-1]
    for offset in range(len(joint_names)):
        if joint_parents is not None:
            parents.append(joint_parents[offset])
        else:
            parents.append(0 if offset == 0 else offset)
    joints = tuple(range(1, len(names)))
    inverse_binds = tuple(
        (binds or {}).get(name, gltf_bind_for(master, name) if master.index_of(name) else translation(7.0, 0.0, 0.0))
        for name in names[1:]
    )
    slots = weighted_slots if weighted_slots is not None else tuple(range(len(joints)))
    return MODULE.SourcePart(
        Path("part.gltf"),
        tuple(names),
        tuple(parents),
        tuple(IDENTITY for _ in names),
        0,
        joints,
        inverse_binds,
        (primitive(slots),),
        ({"name": "part_material"},),
    )


class NormalizePartTest(unittest.TestCase):
    def test_strict_subset_keeps_master_order_rest_and_bind_pose(self) -> None:
        master = master_skeleton()
        part = source_part(master, ("b_root", "b_spine"), joint_parents=(0, 1))
        normalized = MODULE.normalize_part(master, part)

        self.assertEqual(len(normalized.node_names), len(master.bones))
        self.assertEqual(normalized.appended_bones, tuple())
        expected = list(master.names)
        expected[master.mesh_slot_index] = "part_mesh"
        self.assertEqual(list(normalized.node_names), expected)
        self.assertEqual(normalized.node_parents, tuple(bone.parent_index for bone in master.bones))
        # Master rows carry the master's own rest and bind pose, mirrored into
        # glTF space so the cook writes the master values back out.
        for index, bone in enumerate(master.bones):
            self.assertEqual(
                normalized.node_locals[index],
                MODULE.conjugate(bone.rest, MODULE.MIRROR_Z, MODULE.MIRROR_Z),
            )
        slot_of = {node: slot for slot, node in enumerate(normalized.joint_nodes)}
        root_slot = slot_of[master.index_of("b_root")]
        spine_slot = slot_of[master.index_of("b_spine")]
        # Every influence is remapped, including the zero-weight padding slots
        # the extractor leaves pointing at the skin's first joint.
        self.assertEqual(normalized.primitives[0].joints[0], (root_slot,) * 4)
        self.assertEqual(
            normalized.primitives[0].joints[1],
            (spine_slot, root_slot, root_slot, root_slot),
        )
        self.assertEqual(
            normalized.inverse_binds[slot_of[master.index_of("b_spine")]],
            MODULE.conjugate(
                master.bones[master.index_of("b_spine")].inverse_bind,
                MODULE.MIRROR_Z,
                MODULE.MIRROR_Z,
            ),
        )
        self.assertNotIn(master.mesh_slot_index, normalized.joint_nodes)

    def test_geometry_is_rotated_not_rescaled(self) -> None:
        master = master_skeleton()
        part = source_part(master, ("b_root",), joint_parents=(0,))
        normalized = MODULE.normalize_part(master, part)
        source_position = part.primitives[0].positions[0]
        self.assertEqual(
            normalized.primitives[0].positions[0],
            (source_position[0], -source_position[2], source_position[1]),
        )
        self.assertEqual(normalized.primitives[0].directions["NORMAL"][0], (0.0, 0.0, 1.0))

    def test_costume_specific_bones_are_appended_after_the_master(self) -> None:
        master = master_skeleton()
        part = source_part(
            master,
            ("b_root", "b_spine", "b_cloth_01", "b_cloth_02"),
            joint_parents=(0, 1, 2, 3),
        )
        normalized = MODULE.normalize_part(master, part)

        self.assertEqual(normalized.appended_bones, ("b_cloth_01", "b_cloth_02"))
        self.assertEqual(len(normalized.node_names), len(master.bones) + 2)
        self.assertEqual(list(normalized.node_names[: len(master.bones) - 1]), list(master.names[:-1]))
        cloth_one = normalized.node_names.index("b_cloth_01")
        cloth_two = normalized.node_names.index("b_cloth_02")
        self.assertEqual(normalized.node_parents[cloth_one], master.index_of("b_spine"))
        self.assertEqual(normalized.node_parents[cloth_two], cloth_one)
        self.assertTrue(all(parent < index for index, parent in enumerate(normalized.node_parents) if parent >= 0))

    def test_missing_joint_fails_the_part(self) -> None:
        master = master_skeleton()
        # b_orphan is not a master bone and its parent is the part's mesh node,
        # so nothing attaches it to the master palette.
        part = source_part(master, ("b_root", "b_orphan"), joint_parents=(0, 0))
        with self.assertRaisesRegex(MODULE.NormalizeError, "b_orphan"):
            MODULE.normalize_part(master, part)

    def test_duplicate_joint_fails_the_part(self) -> None:
        master = master_skeleton()
        part = source_part(
            master,
            ("b_root", "b_spine"),
            joint_parents=(0, 1),
            duplicate_joint_names=True,
        )
        with self.assertRaisesRegex(MODULE.NormalizeError, "duplicate joint names"):
            MODULE.normalize_part(master, part)

    def test_inverse_bind_mismatch_fails_the_part(self) -> None:
        master = master_skeleton()
        part = source_part(
            master,
            ("b_root", "b_spine"),
            joint_parents=(0, 1),
            binds={"b_spine": translation(9.0, 0.0, 0.0)},
        )
        with self.assertRaisesRegex(MODULE.NormalizeError, "bind pose disagrees with the master at b_spine"):
            MODULE.normalize_part(master, part)

    def test_unweighted_joint_bind_pose_does_not_fail_the_part(self) -> None:
        master = master_skeleton()
        part = source_part(
            master,
            ("b_root", "b_spine"),
            joint_parents=(0, 1),
            binds={"b_spine": translation(9.0, 0.0, 0.0)},
            weighted_slots=(0, 0),
        )
        normalized = MODULE.normalize_part(master, part)
        self.assertEqual(normalized.compared_joint_count, 1)

    def test_mesh_node_name_colliding_with_the_master_fails_the_part(self) -> None:
        master = master_skeleton()
        part = source_part(master, ("b_root",), joint_parents=(0,), mesh_node_name="b_spine")
        with self.assertRaisesRegex(MODULE.NormalizeError, "collides with a master bone"):
            MODULE.normalize_part(master, part)


class MasterSkeletonTest(unittest.TestCase):
    def test_master_requires_one_leaf_mesh_slot_below_the_root(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "body.wmodel"
            # Two leaf children below the root leave the mesh slot ambiguous.
            path.write_bytes(
                build_wmodel(
                    ("RootNode", "leaf_a", "leaf_b"),
                    (-1, 0, 0),
                    (IDENTITY, IDENTITY, IDENTITY),
                    ((0, 0, 0, 0),),
                )
            )
            with self.assertRaisesRegex(MODULE.NormalizeError, "exactly one leaf mesh slot"):
                MODULE.load_master_skeleton("TestClass", path)


def build_wmodel(
    names: tuple[str, ...],
    parents: tuple[int, ...],
    matrices: tuple[tuple[float, ...], ...],
    vertex_joints: tuple[tuple[int, int, int, int], ...],
) -> bytes:
    """Assemble the smallest skinned WModel the readers accept."""
    vertex_stride = 76
    vertices = bytearray()
    for joints in vertex_joints:
        vertices += struct.pack("<11f", *([0.0] * 11))
        vertices += struct.pack("<4I", *joints)
        vertices += struct.pack("<4f", 1.0, 0.0, 0.0, 0.0)
    indices = struct.pack("<3H", 0, 0, 0)

    mesh_payload = bytearray()
    mesh_payload += MODULE.MESH_HEADER.pack(
        b"WMSH", 1, len(names), MODULE.VF_BONE_WEIGHT | 0x0F, vertex_stride,
        len(vertex_joints), 3, 2, 0, b"\0\0\0",
    )
    mesh_payload += MODULE.SUBMESH_DESC.pack(0, len(vertex_joints), 0, 3, 0, 0, b"\0" * 20)
    mesh_payload += vertices
    mesh_payload += indices
    for name, parent, matrix in zip(names, parents, matrices):
        mesh_payload += MODULE.MESH_BONE.pack(
            hash(name) & 0xFFFFFFFF, name.encode("utf-8"), parent, *matrix, 0, b"\0" * 16
        )

    skeleton_payload = bytearray()
    skeleton_payload += MODULE.SKELETON_HEADER.pack(b"WSKL", len(names), 0, 0, 0, 0, 0, 0)
    children: dict[int, list[int]] = {}
    for index, parent in enumerate(parents):
        if parent >= 0:
            children.setdefault(parent, []).append(index)
    for index, (name, parent, matrix) in enumerate(zip(names, parents, matrices)):
        own = children.get(index, [])
        skeleton_payload += MODULE.SKELETON_BONE.pack(
            hash(name) & 0xFFFFFFFF, name.encode("utf-8"), parent, *matrix,
            len(own), min(own) if own else 0xFFFFFFFF, *([0] * 27),
        )

    def wrap(payload: bytes) -> bytes:
        return MODULE.FILE_HEADER.pack(b"WINT", 1, 0, 0, len(payload)) + payload

    sections = [(MODULE.SECTION_MESH, wrap(bytes(mesh_payload))), (MODULE.SECTION_SKELETON, wrap(bytes(skeleton_payload)))]
    table = MODULE.MODEL_HEADER.size + len(sections) * MODULE.SECTION_DESC.size
    content = bytearray(MODULE.MODEL_HEADER.pack(b"WMOD", len(sections), 0, 1, 0, 0, 0, 0))
    body = bytearray()
    descriptors = bytearray()
    for type_id, blob in sections:
        descriptors += MODULE.SECTION_DESC.pack(type_id, 0, table + len(body), len(blob), b"\0" * 40)
        body += blob
    content += descriptors
    content += body
    return MODULE.FILE_HEADER.pack(b"WINT", 1, 0, 0, len(content)) + bytes(content)


class ReorderCookedPaletteTest(unittest.TestCase):
    def test_costume_bone_is_moved_behind_the_master_rows(self) -> None:
        # The cook emits depth-first order, so a costume bone parented to a
        # mid-tree master bone lands before the master rows that follow it.
        cooked_names = ("RootNode", "rig", "b_root", "b_cloth", "b_spine", "part_mesh")
        cooked_parents = (-1, 0, 1, 2, 2, 0)
        matrices = tuple(translation(float(index), 0.0, 0.0) for index in range(len(cooked_names)))
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "part.wmodel"
            path.write_bytes(
                build_wmodel(cooked_names, cooked_parents, matrices, ((3, 4, 0, 0), (2, 2, 2, 2)))
            )
            desired = ("RootNode", "rig", "b_root", "b_spine", "part_mesh", "b_cloth")
            moved = MODULE.reorder_cooked_bone_palette(path, desired)
            self.assertEqual(moved, 3)

            sections = MODULE.read_wmodel_sections(path)
            palette = MODULE.read_mesh_palette(
                [blob for type_id, blob in sections if type_id == MODULE.SECTION_MESH][0], "mesh"
            )
            skeleton = MODULE.read_skeleton_bones(
                [blob for type_id, blob in sections if type_id == MODULE.SECTION_SKELETON][0], "skeleton"
            )
            self.assertEqual(palette.names, desired)
            self.assertEqual(tuple(bone[0] for bone in skeleton), desired)
            self.assertEqual(tuple(bone[1] for bone in skeleton), (-1, 0, 1, 2, 0, 2))
            self.assertTrue(all(bone[1] < index for index, bone in enumerate(skeleton) if bone[1] >= 0))
            # The moved rows keep their own matrices and every blend index follows.
            self.assertEqual(palette.inverse_binds[5], matrices[3])
            self.assertEqual(palette.inverse_binds[3], matrices[4])
            self.assertEqual(palette.weighted_bones, frozenset({5, 2}))

    def test_reorder_rejects_a_target_that_is_not_a_permutation(self) -> None:
        names = ("RootNode", "rig", "b_root", "part_mesh")
        matrices = tuple(IDENTITY for _ in names)
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "part.wmodel"
            path.write_bytes(build_wmodel(names, (-1, 0, 1, 0), matrices, ((0, 0, 0, 0),)))
            with self.assertRaisesRegex(MODULE.NormalizeError, "not a permutation"):
                MODULE.reorder_cooked_bone_palette(path, ("RootNode", "rig", "b_root", "other_mesh"))


class CookCommandTest(unittest.TestCase):
    def test_cook_never_passes_pretransform(self) -> None:
        source = Path(MODULE.__file__).read_text(encoding="utf-8")
        self.assertNotIn('"--pretransform"', source.replace('"--pretransform" not in command', ""))
        self.assertIn('"--scale"', source)
        self.assertIn('"--no-auto-textures"', source)
        self.assertEqual(MODULE.COOK_SCALE, "100")


if __name__ == "__main__":
    unittest.main()
