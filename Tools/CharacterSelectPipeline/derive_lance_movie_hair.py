"""Derive an intentional FT43 hairstyle for the existing Lance Movie actor.

Keep the Movie's skeleton prefix and cinematic keys, preserve every donor
positive weight, and append the donor's otherwise absent hair chain. This is
an appearance change, not a claim that the original FT06 asset was corrupt.
Outputs are candidates under out; installation is a separate reviewed step.
"""
from __future__ import annotations

import argparse
from dataclasses import replace
import hashlib
import json
from pathlib import Path
import struct
import sys

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Tools/ModelAssetConverter"))
import cook_wmodel_geometry_contract as geom
import verify_dimensionmaster_summon_bind_pose as wm
from bake_guardian_selection import rebase_donor_material_paths

DONOR = "Character/LanceMaster/Equipment/pc_ft_43_hair/head.wmodel"
MOVIE = "Character/LanceMaster/Cinematics/ClassSelect/Original/Parts/fighter_hair.slot0.wmodel"
TARGET = "Character/LanceMaster/Cinematics/ClassSelect/Appearance/FT43_Hair.wmodel"
BASIS = np.array([[1., 0., 0., 0.], [0., 0., -1., 0.],
                  [0., -1., 0., 0.], [0., 0., 0., 1.]])


def digest(data):
    return hashlib.sha256(data).hexdigest()


def matrix(values):
    return (BASIS @ np.array(values).reshape(4, 4) @ BASIS).ravel().tolist()


def nested(payload):
    result = bytearray(payload)
    header = list(wm.FILE_HEADER.unpack_from(result))
    header[-1] = len(result) - wm.FILE_HEADER.size
    wm.FILE_HEADER.pack_into(result, 0, *header)
    return bytes(result)


def derive(resources: Path, output: Path):
    donor_path, movie_path = resources / DONOR, resources / MOVIE
    donor_bytes, movie_bytes = donor_path.read_bytes(), movie_path.read_bytes()
    donor = wm.read_wmodel(donor_path, animation_names=())
    movie = wm.read_wmodel(movie_path, animation_names=())
    dp = geom.parse_skinned_uv_wmodel(rebase_donor_material_paths(donor_bytes, donor_path, resources))
    mp = geom.parse_skinned_uv_wmodel(movie_bytes)
    donor_by_name = {b.name: i for i, b in enumerate(donor.skeleton_bones)}
    target_names = [b.name for b in movie.skeleton_bones]
    missing = set()
    positive_influences = 0
    for vertex in donor.vertices:
        for index, weight in zip(vertex.indices, vertex.weights):
            if weight <= 0:
                continue
            positive_influences += 1
            while donor.skeleton_bones[index].name not in target_names:
                missing.add(index)
                index = donor.skeleton_bones[index].parent
                assert index >= 0, "Donor weighted ancestry has no Movie parent"
    # Preserve the complete terminal hair chain even though its tip is unweighted.
    for index, bone in enumerate(donor.skeleton_bones):
        if bone.name.startswith("b_add_hair01_b_"):
            missing.add(index)
    extras = sorted(missing)
    assert [donor.skeleton_bones[i].name for i in extras] == [
        "b_add_hair01_b_11", "b_add_hair01_b_12", "b_add_hair01_b_13", "b_add_hair01_b_14"]
    target_names += [donor.skeleton_bones[i].name for i in extras]
    target_index = {name: i for i, name in enumerate(target_names)}
    parents = [b.parent for b in movie.skeleton_bones]
    for index in extras:
        parents.append(target_index[donor.skeleton_bones[donor.skeleton_bones[index].parent].name])
    assert all(parent < index for index, parent in enumerate(parents))

    # Geometry is in installed legacy (X,Y,Z); Movie uses (X,-Z,-Y).
    mesh = bytearray(dp["mesh"])
    h = list(dp["meshHeader"])
    vertex_start, index_start = dp["vertexStart"], dp["indexStart"]
    for index, vertex in enumerate(donor.vertices):
        at = vertex_start + index * h[4]
        for field in (0, 12, 32):
            x, y, z = struct.unpack_from("<3f", mesh, at + field)
            struct.pack_into("<3f", mesh, at + field, x, -z, -y)
        if h[4] == 80:
            value = struct.unpack_from("<f", mesh, at + 76)[0]
            struct.pack_into("<f", mesh, at + 76, -value)
        mapped = [target_index[donor.skeleton_bones[i].name] if w > 0 else 0
                  for i, w in zip(vertex.indices, vertex.weights)]
        struct.pack_into("<4I", mesh, at + 44, *mapped)
        assert struct.unpack_from("<4f", mesh, at + 60) == vertex.weights
    code = "<H" if h[7] == 2 else "<I"
    for desc in dp["submeshes"]:
        for index in range(0, desc[3], 3):
            a = index_start + desc[2] + index * h[7]
            b = a + 2 * h[7]
            first, last = struct.unpack_from(code, mesh, a)[0], struct.unpack_from(code, mesh, b)[0]
            struct.pack_into(code, mesh, a, last)
            struct.pack_into(code, mesh, b, first)
    bone_start = index_start + h[6] * h[7]
    bounds_start = bone_start + h[2] * wm.MESH_BONE.size
    bounds_end = bounds_start + (h[1] * geom.BOUNDS_V1.size if h[8] else 0)
    bones = bytearray()
    target_hashes = []
    for index, name in enumerate(target_names):
        original = donor.mesh_bones[donor_by_name[name]] if name in donor_by_name else movie.mesh_bones[index]
        transform = matrix(original.transform) if name in donor_by_name else original.transform
        target_hashes.append(original.name_hash)
        bones.extend(wm.MESH_BONE.pack(original.name_hash, name.encode().ljust(32, b"\0"),
            parents[index], *transform, 0, bytes(16)))
    bounds = bytearray(mesh[bounds_start:bounds_end])
    if h[8]:
        for index, desc in enumerate(dp["submeshes"]):
            positions = np.array([struct.unpack_from("<3f", mesh, vertex_start + desc[0] + i * h[4])
                                  for i in range(desc[1])])
            low, high = positions.min(axis=0), positions.max(axis=0)
            center = (low + high) * .5
            radius = np.linalg.norm(positions - center, axis=1).max()
            geom.BOUNDS_V1.pack_into(bounds, index * geom.BOUNDS_V1.size, *low, *high, *center, radius)
    mesh = mesh[:bone_start] + bones + bounds + mesh[bounds_end:]
    h[2] = len(target_names)
    wm.MESH_HEADER.pack_into(mesh, 16, *h)
    mesh = nested(mesh)

    # Keep the entire original rest-pose prefix and all socket/root data.
    original_skeleton = next(s.payload for s in mp["sections"] if s.type_id == 3)
    donor_skeleton = next(s.payload for s in dp["sections"] if s.type_id == 3)
    sh = list(wm.SKELETON_HEADER.unpack_from(original_skeleton, 16))
    start = 16 + wm.SKELETON_HEADER.size
    nodes = [list(wm.SKELETON_BONE.unpack_from(original_skeleton, start + i * wm.SKELETON_BONE.size))
             for i in range(sh[1])]
    for source_index in extras:
        node = list(wm.SKELETON_BONE.unpack_from(donor_skeleton, start + source_index * wm.SKELETON_BONE.size))
        node[2] = parents[len(nodes)]
        node[3:19] = matrix(node[3:19])
        nodes.append(node)
    for index, node in enumerate(nodes):
        children = [i for i, parent in enumerate(parents) if parent == index]
        node[19], node[20] = len(children), children[0] if children else 0
    trailer = original_skeleton[start + sh[1] * wm.SKELETON_BONE.size:]
    sh[1] = len(nodes)
    skeleton = nested(original_skeleton[:16] + wm.SKELETON_HEADER.pack(*sh) +
                      b"".join(wm.SKELETON_BONE.pack(*node) for node in nodes) + trailer)
    skeleton_hash = 0xcbf29ce484222325
    for value in target_hashes:
        skeleton_hash = ((skeleton_hash ^ value) * 0x100000001b3) & ((1 << 64) - 1)
    sections = []
    material = next(s for s in dp["sections"] if s.type_id == 2)
    clips = []
    for section in mp["sections"]:
        payload = section.payload
        if section.type_id == 1:
            payload = mesh
        elif section.type_id == 2:
            payload = material.payload
        elif section.type_id == 3:
            payload = skeleton
        elif section.type_id == 4:
            payload = payload[:-8] + struct.pack("<Q", skeleton_hash)
            assert payload[:-8] == section.payload[:-8]
            ah = wm.ANIMATION_HEADER.unpack_from(payload, 16)
            clips.append(dict(name=section.name_bytes.split(b"\0")[0].decode(),
                durationTicks=ah[2], ticksPerSecond=ah[3], channels=ah[1],
                originalKeysByteIdentical=True))
        sections.append(replace(section, payload=payload))
    result = geom.rebuild_wmodel(mp["modelHeader"], sections, mesh)
    geom.parse_skinned_uv_wmodel(result)
    output.parent.mkdir(parents=True, exist_ok=True)
    assert not output.exists(), "Use a fresh candidate path"
    output.write_bytes(result)
    parsed = wm.read_wmodel(output, animation_names=())
    assert len(parsed.vertices) == len(donor.vertices)
    for old, new in zip(donor.vertices, parsed.vertices):
        assert old.weights == new.weights
        for oi, ni, weight in zip(old.indices, new.indices, old.weights):
            if weight > 0:
                assert donor.skeleton_bones[oi].name == parsed.skeleton_bones[ni].name
    return dict(intent="User-requested fuller FT43 appearance; not source restoration",
        donor=DONOR, donorSha256=digest(donor_bytes), movie=MOVIE, movieSha256=digest(movie_bytes),
        targetAssetId=TARGET, output=str(output), outputSha256=digest(result),
        vertexCount=len(parsed.vertices), skeletonCount=len(nodes), moviePrefixBones=len(movie.skeleton_bones),
        extraBones=target_names[len(movie.skeleton_bones):], positiveInfluencesPreserved=positive_influences,
        droppedPositiveInfluences=0, weightsByteIdentical=True, clips=clips,
        basis=BASIS.tolist(), modelPreScale=.01,
        boundary="Extra hair chain follows animated neck with its original local rest; no invented secondary motion.")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--receipt", type=Path, required=True)
    args = parser.parse_args()
    for path in (args.output, args.receipt):
        assert path.resolve().is_relative_to((ROOT / "out").resolve())
    receipt = derive(ROOT / "Client/Bin/Resources", args.output.resolve())
    args.receipt.write_text(json.dumps(receipt, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(receipt, ensure_ascii=False))


if __name__ == "__main__":
    main()
