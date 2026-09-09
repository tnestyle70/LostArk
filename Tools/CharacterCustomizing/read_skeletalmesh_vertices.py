"""Read a retail USkeletalMesh3's GPU vertex buffer (position + UV0) straight out of a
LostArk ``.upk``, without going through UModel.

Why this exists: UModel (every build available to this project, ``_v7``, ``_custom`` and
``_rawdump``) fails to export a PSK for these four classes' face meshes -- it gets through
the tagged-property list fine (those are self-describing and any build can skip an unknown
one by its declared size) but throws inside ``FStaticLODModel3``'s native array
serialization, which apparently has a struct layout UModel's bundled parser doesn't
recognise for this game (confirmed even on a class whose retail package predates any of the
client patches this project has looked at, so it is not a "which patch" question).

Hand-deriving that struct's exact byte layout turned out to be unnecessary. The one number
this tool actually needs -- ``NumVertices`` -- is already known independently for every class
from ``build_face_morphs.py``'s own ``.facemorphs`` output (its ``vertexIndexBound``, which is
this same value read straight out of the export earlier in the same struct by a different,
already-working code path). So instead of walking ``FSkelMeshSection3``/``FSkelIndexBuffer3``/
``FSkelMeshChunk3`` (each of which has extra, per-asset-inconsistent bytes appended in at
least one of the four classes -- confirmed by hand for two of them, not fully explained),
this tool searches the raw export for the one place a ``[stride:int32][count:int32]`` pair is
immediately followed by ``count`` consecutive ``stride``-byte records whose first three floats
(at a fixed +16 byte offset within the record -- the same offset independently confirmed
against a real exported PSK for one class, see ``FACEMORPH_FORMAT.md``) look like real,
head-sized 3D positions. Both ``count == NumVertices`` and "looks like a real position" have
to hold for three consecutive records, which in practice has produced exactly one match per
class -- checked against a real exported PSK for the one class where one exists (byte-for-byte
identical, 1884/1884 vertices, worst delta 0.0) and cross-validated for the other three by the
uniqueness of the match itself plus a sane resulting bounding box.

usage (as a library):
    from read_skeletalmesh_vertices import read_vertices
    verts = read_vertices(upk_path, "pc_wr_00_face_sk", expect_count=1362)
    # -> list of ((x, y, z), (u, v))
"""
from __future__ import annotations

import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import build_face_morphs as _bfm  # noqa: E402  (decompress_package / read_names / read_exports)

PLAUSIBLE_STRIDES = (24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64)
POSITION_SANITY_LIMIT = 300.0  # source units (cm); a real head is a few tens of cm across
POSITION_FIELD_OFFSET = 16     # FGPUVert3{Half,Float}: 16B common (2 packed normals + bone
                               # index/weight) precede the FVector position
UV_FIELD_OFFSET = 28
SANITY_SAMPLE = 3


def _find_export(buffer, names, exports, object_name: str):
    matches = [e for e in exports
              if e["name"] == object_name and e["className"].lower() == "skeletalmesh"]
    if len(matches) != 1:
        raise SystemExit("%s: expected exactly one skeletalmesh export named %r, found %d"
                         % (object_name, object_name, len(matches)))
    return matches[0]


def _find_vertex_buffer(buffer, start: int, end: int, expect_count: int):
    hits = []
    for candidate in range(start, end - 8):
        stride, count = struct.unpack_from("<2i", buffer, candidate)
        if count != expect_count or stride not in PLAUSIBLE_STRIDES:
            continue
        vertex_start = candidate + 8
        if vertex_start + stride * SANITY_SAMPLE > end:
            continue
        sane = True
        for i in range(SANITY_SAMPLE):
            x, y, z = struct.unpack_from("<3f", buffer, vertex_start + i * stride + POSITION_FIELD_OFFSET)
            if not all(abs(c) < POSITION_SANITY_LIMIT for c in (x, y, z)):
                sane = False
                break
        if sane:
            hits.append((vertex_start, stride))
    return hits


def read_vertices(upk_path: Path, object_name: str, expect_count: int):
    """Return a list of ``((x, y, z), (u, v))``, one per GPU vertex buffer entry, in the
    same order ``build_face_morphs.py``'s ``.facemorphs`` ``vertexIndex`` addresses."""
    buffer, info = _bfm.decompress_package(upk_path)
    names = _bfm.read_names(buffer, info)
    exports = _bfm.read_exports(buffer, info, names)
    export = _find_export(buffer, names, exports, object_name)

    hits = _find_vertex_buffer(buffer, export["offset"], export["offset"] + export["size"], expect_count)
    if len(hits) != 1:
        raise SystemExit("%s: expected exactly one vertex buffer candidate at count=%d, found %d: %s"
                         % (object_name, expect_count, len(hits), hits))
    vertex_start, stride = hits[0]

    vertices = []
    for i in range(expect_count):
        base = vertex_start + i * stride
        position = struct.unpack_from("<3f", buffer, base + POSITION_FIELD_OFFSET)
        uv = struct.unpack_from("<2e", buffer, base + UV_FIELD_OFFSET)
        vertices.append((position, uv))
    return vertices


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("upk", type=Path)
    parser.add_argument("object_name")
    parser.add_argument("expect_count", type=int)
    args = parser.parse_args()

    verts = read_vertices(args.upk, args.object_name, args.expect_count)
    xs = [p[0][0] for p in verts]
    ys = [p[0][1] for p in verts]
    zs = [p[0][2] for p in verts]
    print("%d vertices, bbox x[%.2f, %.2f] y[%.2f, %.2f] z[%.2f, %.2f]"
          % (len(verts), min(xs), max(xs), min(ys), max(ys), min(zs), max(zs)))
