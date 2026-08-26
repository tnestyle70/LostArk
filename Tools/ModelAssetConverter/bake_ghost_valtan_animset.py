"""Bake a Ghost-Valtan-compatible animation donor from the product Valtan AnimSet.

Why this exists
---------------
The ghost body (MN_RPBF_02) already carries the same motions as the product
Valtan, but under its own clip vocabulary (`rpbf_02.ao_*`) and at 1/100 the
authored scale. Anyone authoring a chain against the product body therefore has
to remember a rename rule before it plays on the ghost.

This tool removes that rule instead of documenting it: it rewrites the product
AnimSet into a package the ghost body accepts through
`CModel::Attach_AnimationSet`, keeping the product `mesh_*` clip names. After
that both bodies answer to one vocabulary and a chain authored on either plays
on the other unchanged.

Two things block a straight copy, and this tool fixes exactly those two:

1. `Attach_AnimationSet` requires equal skeleton hashes. The hash is FNV-1a over
   the bone name hashes, and two of the 87 bones differ -- the exporter's mesh
   container nodes, `mesh`/`mesh.001` against `rpbf_02.ao`/`rpbf_02.mo`. The
   other 85 bones already share both name and hash.
2. Every animated translation is exactly 100x the ghost's, because the two
   bodies were exported at different unit scales. Applied unscaled, the bone
   origins would spread 100x apart while the skinned mesh stayed ghost-sized.

The package's own skeleton and mesh blocks are left at product scale on purpose.
`Attach_AnimationSet` copies animations only, so those blocks exist here just to
satisfy the container's own validation and never reach skinning. Rewriting them
would add risk without changing a pixel.

Usage
-----
    py Tools/ModelAssetConverter/bake_ghost_valtan_animset.py [--check]

`--check` validates and reports without writing.
"""

from __future__ import annotations

import argparse
import hashlib
import struct
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
RESOURCES = REPO / "Client/Bin/Resources/Character/Valtan"
SOURCE = RESOURCES / "AnimSets/MN_RPBF_01_AnimSet.wmodel"
REFERENCE = RESOURCES / "Ghost/MN_RPBF_02.wmodel"
DESTINATION = RESOURCES / "Ghost/MN_RPBF_02_AnimSet.wmodel"

# Engine/private/BinaryAsset/Winters/WFormatTypes.h
FILE_HEADER_SIZE = 16
MODEL_META_SIZE = 32
SECTION_DESC_SIZE = 64
SKELETON_META_SIZE = 32
SKELETON_BONE_NODE_SIZE = 256
GLOBAL_ROOT_MATRIX_SIZE = 128
SOCKET_ENTRY_SIZE = 128
MESH_META_SIZE = 36
SUBMESH_DESC_SIZE = 48
MESH_BONE_ENTRY_SIZE = 128
ANIMATION_META_SIZE = 32
ANIMATION_CHANNEL_SIZE = 40
VECTOR_KEY_SIZE = 16
ANIMATION_EVENT_SIZE = 32
ANIMATION_TRAILER_SIZE = 8

SECTION_MESH = 1
SECTION_SKELETON = 3
SECTION_ANIMATION = 4

# The ghost is authored at 1/100 of the product, measured from both the bind
# pose and every root-motion curve the two bodies share.
TRANSLATION_SCALE = 0.01


class BakeError(RuntimeError):
    pass


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1 << 20), b""):
            digest.update(block)
    return digest.hexdigest()


def read_sections(data: bytes) -> tuple[int, list[dict]]:
    magic, major, _minor, flags, content_size = struct.unpack_from(
        "<4sHHII", data, 0
    )
    if magic != b"WINT" or major != 1 or flags != 0:
        raise BakeError("outer WINT header is invalid")
    if content_size != len(data) - FILE_HEADER_SIZE:
        raise BakeError("outer WINT content size does not match the file")

    model_at = FILE_HEADER_SIZE
    model_magic, section_count, animation_count, _model_flags = struct.unpack_from(
        "<4sIII", data, model_at
    )
    if model_magic != b"WMOD":
        raise BakeError("WMOD metadata is missing")

    sections = []
    table = model_at + MODEL_META_SIZE
    for index in range(section_count):
        desc_at = table + index * SECTION_DESC_SIZE
        section_type, section_index, offset, size = struct.unpack_from(
            "<IIQQ", data, desc_at
        )
        name = data[desc_at + 24: desc_at + SECTION_DESC_SIZE].split(b"\0")[0]
        sections.append(
            {
                "type": section_type,
                "index": section_index,
                "offset": offset,
                "size": size,
                "name": name.decode("ascii", "replace"),
                "desc_at": desc_at,
                # Every section carries its own WINT header before its payload.
                "payload_at": FILE_HEADER_SIZE + offset + FILE_HEADER_SIZE,
            }
        )
    return animation_count, sections


def skeleton_bones(data: bytes, sections: list[dict]) -> list[dict]:
    skeleton = [s for s in sections if s["type"] == SECTION_SKELETON]
    if len(skeleton) != 1:
        raise BakeError("package must hold exactly one skeleton section")
    base = skeleton[0]["payload_at"]
    magic, bone_count, socket_count = struct.unpack_from("<4sII", data, base)
    if magic != b"WSKL":
        raise BakeError("skeleton section is not WSKL")
    bones = []
    for index in range(bone_count):
        at = base + SKELETON_META_SIZE + index * SKELETON_BONE_NODE_SIZE
        name_hash, raw_name = struct.unpack_from("<Q64s", data, at)
        bones.append(
            {
                "index": index,
                "name_hash": name_hash,
                "name": raw_name.split(b"\0")[0].decode("ascii", "replace"),
                "at": at,
            }
        )
    return bones


def mesh_bones(data: bytes, sections: list[dict]) -> list[dict]:
    mesh = [s for s in sections if s["type"] == SECTION_MESH]
    if len(mesh) != 1:
        raise BakeError("package must hold exactly one mesh section")
    base = mesh[0]["payload_at"]
    (
        magic,
        submesh_count,
        bone_count,
        _vertex_format,
        vertex_stride,
        total_vertices,
        total_indices,
        index_stride,
        _has_bounding,
    ) = struct.unpack_from("<4sIIIIIIIB", data, base)
    if magic != b"WMSH":
        raise BakeError("mesh section is not WMSH")
    # WMeshReader order: header, submesh descriptors, vertex blob, index blob,
    # then the bone table. The blobs are skipped by size, not parsed.
    table = (
        base
        + MESH_META_SIZE
        + submesh_count * SUBMESH_DESC_SIZE
        + total_vertices * vertex_stride
        + total_indices * index_stride
    )
    bones = []
    for index in range(bone_count):
        at = table + index * MESH_BONE_ENTRY_SIZE
        name_hash, raw_name = struct.unpack_from("<Q32s", data, at)
        bones.append(
            {
                "index": index,
                "name_hash": name_hash,
                "name": raw_name.split(b"\0")[0].decode("ascii", "replace"),
                "at": at,
            }
        )
    return bones


def skeleton_hash(bones: list[dict]) -> int:
    value = 0xCBF29CE484222325
    for bone in bones:
        value ^= bone["name_hash"]
        value = (value * 0x100000001B3) & 0xFFFFFFFFFFFFFFFF
    return value


def plan_bone_rewrites(source_bones, reference_bones) -> list[dict]:
    if len(source_bones) != len(reference_bones):
        raise BakeError(
            f"bone counts differ: source {len(source_bones)} "
            f"reference {len(reference_bones)}"
        )
    rewrites = []
    for source, reference in zip(source_bones, reference_bones):
        if source["name"] == reference["name"]:
            if source["name_hash"] != reference["name_hash"]:
                raise BakeError(
                    f"bone {source['index']} shares a name but not a hash; "
                    "the two rigs are not the same skeleton"
                )
            continue
        rewrites.append(
            {
                "index": source["index"],
                "from_name": source["name"],
                "from_hash": source["name_hash"],
                "to_name": reference["name"],
                "to_hash": reference["name_hash"],
            }
        )
    return rewrites


def bake(check_only: bool) -> int:
    for path in (SOURCE, REFERENCE):
        if not path.is_file():
            raise BakeError(f"missing input: {path}")

    source = bytearray(SOURCE.read_bytes())
    reference = REFERENCE.read_bytes()

    animation_count, sections = read_sections(bytes(source))
    _ref_animation_count, ref_sections = read_sections(reference)

    source_bones = skeleton_bones(bytes(source), sections)
    reference_bones = skeleton_bones(reference, ref_sections)
    rewrites = plan_bone_rewrites(source_bones, reference_bones)

    target_hash = skeleton_hash(reference_bones)
    print(f"source        : {SOURCE.relative_to(REPO)}")
    print(f"reference rig : {REFERENCE.relative_to(REPO)}")
    print(f"bones         : {len(source_bones)}   animations: {animation_count}")
    print(f"skeleton hash : 0x{skeleton_hash(source_bones):016x} "
          f"-> 0x{target_hash:016x}")
    for rewrite in rewrites:
        print(f"  bone[{rewrite['index']}] {rewrite['from_name']!r} "
              f"-> {rewrite['to_name']!r}")
    if not rewrites:
        raise BakeError("no bone differs; the source already matches the rig")

    hash_map = {r["from_hash"]: r["to_hash"] for r in rewrites}

    # 1. Skeleton bone identities.
    for rewrite in rewrites:
        at = source_bones[rewrite["index"]]["at"]
        struct.pack_into("<Q", source, at, rewrite["to_hash"])
        struct.pack_into("<64s", source, at + 8, rewrite["to_name"].encode("ascii"))

    # 2. The mesh bone table the decoder cross-checks against the skeleton.
    source_mesh_bones = mesh_bones(bytes(source), sections)
    if len(source_mesh_bones) != len(source_bones):
        raise BakeError("mesh and skeleton bone counts disagree")
    mesh_rewrites = 0
    for rewrite in rewrites:
        entry = source_mesh_bones[rewrite["index"]]
        if entry["name_hash"] != rewrite["from_hash"]:
            raise BakeError(
                f"mesh bone {rewrite['index']} does not carry the skeleton hash"
            )
        struct.pack_into("<Q", source, entry["at"], rewrite["to_hash"])
        struct.pack_into(
            "<32s", source, entry["at"] + 8, rewrite["to_name"].encode("ascii")
        )
        mesh_rewrites += 1

    # 3. Animation channels: bone identity, translation scale, skeleton trailer.
    scaled_keys = 0
    retagged_channels = 0
    animations = 0
    for section in sections:
        if section["type"] != SECTION_ANIMATION:
            continue
        animations += 1
        base = section["payload_at"]
        magic, channel_count, duration, ticks, total_keys, event_count, _loop = (
            struct.unpack_from("<4sIffIIB", source, base)
        )
        if magic != b"WANM":
            raise BakeError(f"animation {section['name']} is not WANM")

        channel_table = base + ANIMATION_META_SIZE
        key_block = channel_table + channel_count * ANIMATION_CHANNEL_SIZE
        section_end = FILE_HEADER_SIZE + section["offset"] + section["size"]
        trailer_at = section_end - ANIMATION_TRAILER_SIZE
        key_block_size = (
            trailer_at - event_count * ANIMATION_EVENT_SIZE - key_block
        )
        if key_block_size < 0:
            raise BakeError(f"animation {section['name']} key block is truncated")

        for channel in range(channel_count):
            entry = channel_table + channel * ANIMATION_CHANNEL_SIZE
            bone_hash, position_count, position_offset = struct.unpack_from(
                "<QII", source, entry
            )
            if bone_hash in hash_map:
                struct.pack_into("<Q", source, entry, hash_map[bone_hash])
                retagged_channels += 1

            span = position_count * VECTOR_KEY_SIZE
            if position_offset + span > key_block_size:
                raise BakeError(
                    f"animation {section['name']} position span leaves its key block"
                )
            for key in range(position_count):
                key_at = key_block + position_offset + key * VECTOR_KEY_SIZE
                time_ticks, x, y, z = struct.unpack_from("<4f", source, key_at)
                struct.pack_into(
                    "<4f",
                    source,
                    key_at,
                    time_ticks,
                    x * TRANSLATION_SCALE,
                    y * TRANSLATION_SCALE,
                    z * TRANSLATION_SCALE,
                )
                scaled_keys += 1

        struct.pack_into("<Q", source, trailer_at, target_hash)

    print(f"rewrote       : {len(rewrites)} skeleton bones, "
          f"{mesh_rewrites} mesh bones")
    print(f"animations    : {animations}")
    print(f"channels retagged : {retagged_channels}")
    print(f"position keys scaled x{TRANSLATION_SCALE} : {scaled_keys}")

    # Re-read the product to prove only the intended bytes moved.
    original = SOURCE.read_bytes()
    if len(original) != len(source):
        raise BakeError("bake changed the package length")
    changed = sum(1 for a, b in zip(original, source) if a != b)
    print(f"bytes changed : {changed} of {len(source)}")

    verify_bones = skeleton_bones(bytes(source), sections)
    if skeleton_hash(verify_bones) != target_hash:
        raise BakeError("baked skeleton hash does not match the rig")
    print(f"verified hash : 0x{skeleton_hash(verify_bones):016x}")

    if check_only:
        print("\n--check: nothing written.")
        return 0

    DESTINATION.parent.mkdir(parents=True, exist_ok=True)
    temporary = DESTINATION.with_suffix(".wmodel.tmp")
    temporary.write_bytes(bytes(source))
    temporary.replace(DESTINATION)
    print(f"\nwrote {DESTINATION.relative_to(REPO)}")
    print(f"  bytes  {DESTINATION.stat().st_size}")
    print(f"  sha256 {sha256(DESTINATION)}")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--check",
        action="store_true",
        help="validate and report without writing the package",
    )
    args = parser.parse_args()
    try:
        return bake(args.check)
    except BakeError as error:
        print(f"bake failed: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
