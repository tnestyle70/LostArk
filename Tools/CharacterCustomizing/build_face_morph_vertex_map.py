"""Map a class' retail face-morph source vertex indices onto its runtime .wmodel.

``build_face_morphs.py`` writes ``<Class>.facemorphs`` deltas indexed by the retail
``MorphTarget``'s ``SourceIdx``, which is the retail *wedge* index (one entry per
``VTXW0000`` record of the source PSK), not a position and not a runtime vertex
index. The cooked ``.wmodel`` re-tessellates the mesh (it splits seam/UV-boundary
vertices differently and the wmodel commonly carries more total vertices than the
source has wedges), so a morph delta cannot be applied by ``vertexIndex`` alone --
this tool produces the missing wedge -> runtime-vertex correspondence.

The retail PSK and the cooked wmodel turn out to share the same units and the same
axes up to one sign (x, y, -z, scale 1:1) -- verified per class by brute-forcing
every axis permutation/sign/scale combination against a position sample and keeping
the only one with zero residual. Because of that, matching is a closed problem: for
every source wedge, transform its PSK point and look up every runtime vertex sitting
at the *exact* same position (a small hash-grid neighbour search, not a nearest-point
guess). Ties (several runtime vertices at one position, which happens at multi-
material submesh seams) are broken by UV, trying both facings (v and 1-v) since the
two conversions do not agree on which one is "up". Remaining ties are kept as-is and
every one of them receives the same delta -- they share both position and UV, so they
are the same logical vertex duplicated across a submesh boundary, and the cooker
asserts that every wedge finds at least one match and every runtime vertex is claimed
by at least one wedge.

Two ways to get the source (points, wedges): a real exported ``.psk`` (``--psk``), or straight
from the retail ``.upk`` (``--upk --object-name --expect-verts``) via
``read_skeletalmesh_vertices.py``, for the classes whose patched packages UModel cannot export
a PSK from at all -- see that module's docstring for why this is trustworthy without one.

usage:
  python build_face_morph_vertex_map.py --psk <face.psk> --wmodel <Class.wmodel>
                                        --mesh-name <submesh name> --class-id <Class>
                                        [--resources <Client/Bin/Resources>]
  python build_face_morph_vertex_map.py --upk <face.upk> --object-name <pc_xx_00_face_sk>
                                        --expect-verts <N> --wmodel <Class.wmodel>
                                        --mesh-name <submesh name> --class-id <Class>
                                        [--resources <Client/Bin/Resources>]
"""
from __future__ import annotations

import argparse
import struct
import sys
from collections import defaultdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import read_skeletalmesh_vertices  # noqa: E402

PSK_CHUNK = struct.Struct("<20siii")

WMODEL_FILE_HEADER = struct.Struct("<4sHHII")
WMODEL_MODEL_HEADER = struct.Struct("<4sIII4I")
WMODEL_SECTION_DESC = struct.Struct("<IIQQ40s")
WMODEL_MESH_HEADER = struct.Struct("<4sIIIIIIIB3s")
WMODEL_SUBMESH_DESC = struct.Struct("<IIIIIQ20s")
WMODEL_SECTION_MESH = 1
WMODEL_SECTION_MATERIAL = 2
WMODEL_SUBMESH_MATERIAL_FIELD = 4
WMODEL_MATERIAL_META = struct.Struct("<4sI")
WMODEL_MATERIAL_ENTRY = {
    b"WMAT": struct.Struct("<IQ64s520s"),
    b"WMA2": struct.Struct("<IQ64s" + "520s" * 9),
    b"WMA3": struct.Struct("<IQ64s" + "520s" * 10 + "16f"),
}

FACEMORPHMAP_MAGIC = b"LAFMVMAP"
FACEMORPHMAP_VERSION = 2

EXACT_EPSILON_SQ = 1e-8
UV_TIE_EPSILON_SQ = 1e-6


# --------------------------------------------------------------------- materials

def read_material_names(path: Path):
    """Return {materialIndex: lowercase name} as a submesh descriptor indexes them.

    Which submesh is skin and which is an eyeball is not something to infer from vertex
    counts: the model says so, in the material each submesh draws with."""
    data = path.read_bytes()
    base = WMODEL_FILE_HEADER.size
    _magic, section_count = WMODEL_MODEL_HEADER.unpack_from(data, base)[:2]
    for index in range(section_count):
        section_type, _i, offset, _size, _name = WMODEL_SECTION_DESC.unpack_from(
            data, base + WMODEL_MODEL_HEADER.size + index * WMODEL_SECTION_DESC.size)
        if section_type != WMODEL_SECTION_MATERIAL:
            continue
        at = base + offset + WMODEL_FILE_HEADER.size
        magic, count = WMODEL_MATERIAL_META.unpack_from(data, at)
        if magic not in WMODEL_MATERIAL_ENTRY:
            raise SystemExit("%s: unknown material container %r" % (path, magic))
        entry = WMODEL_MATERIAL_ENTRY[magic]
        at += WMODEL_MATERIAL_META.size
        names = {}
        for row in range(count):
            fields = entry.unpack_from(data, at + row * entry.size)
            names[fields[0]] = fields[2].split(b"\0")[0].decode("ascii", "replace").lower()
        return names
    raise SystemExit("%s: no material section" % path)


def read_submesh_materials(path: Path):
    """Return the material index each submesh draws with, in submesh order."""
    data = path.read_bytes()
    base = WMODEL_FILE_HEADER.size
    _magic, section_count = WMODEL_MODEL_HEADER.unpack_from(data, base)[:2]
    for index in range(section_count):
        section_type, _i, offset, _size, _name = WMODEL_SECTION_DESC.unpack_from(
            data, base + WMODEL_MODEL_HEADER.size + index * WMODEL_SECTION_DESC.size)
        if section_type != WMODEL_SECTION_MESH:
            continue
        start = next(c for c in (base + offset, offset) if data[c:c + 4] == b"WINT")
        payload = start + WMODEL_FILE_HEADER.size
        submesh_count = WMODEL_MESH_HEADER.unpack_from(data, payload)[1]
        submesh_base = payload + WMODEL_MESH_HEADER.size
        return [WMODEL_SUBMESH_DESC.unpack_from(
            data, submesh_base + row * WMODEL_SUBMESH_DESC.size)[WMODEL_SUBMESH_MATERIAL_FIELD]
            for row in range(submesh_count)]
    raise SystemExit("%s: no mesh section" % path)


# -------------------------------------------------------------------------- PSK

def read_psk(path: Path):
    """Return (points, wedges) with wedges as (point_index, u, v, material)."""
    data = path.read_bytes()
    chunks = {}
    at = 0
    while at + PSK_CHUNK.size <= len(data):
        cid, _flag, size, count = PSK_CHUNK.unpack_from(data, at)
        cid = cid.split(b"\0", 1)[0].decode("ascii", "replace")
        at += PSK_CHUNK.size
        chunks[cid] = (size, count, data[at:at + size * count])
        at += size * count

    stride, count, body = chunks["PNTS0000"]
    points = [struct.unpack_from("<3f", body, i * stride) for i in range(count)]

    stride, count, body = chunks["VTXW0000"]
    wedges = []
    for i in range(count):
        if stride == 16:
            point_index, u, v, material = struct.unpack_from("<Hxx2fB3x", body, i * stride)
        else:
            point_index, u, v, material = struct.unpack_from("<I2fB3x", body, i * stride)
        wedges.append((point_index, u, v, material))
    return points, wedges


# ----------------------------------------------------------------------- wmodel

def read_wmodel_mesh(path: Path, mesh_name: str):
    """Return (vertices, submeshes) for the named mesh section.

    ``vertices`` is the whole WMSH's flat per-vertex (position, uv) list. ``submeshes`` is
    each ``SUBMESH_DESC`` as (vertexIndexOffset, vertexCount, materialIndex) -- the file's own
    ``vertexOffset`` is in bytes, converted here to a vertex index (divided by ``stride``) --
    in file order, the same order --
    the same order ``Engine/Private/BinaryAsset/Winters/WMeshReader.cpp`` slices the flat
    vertex blob into separate ``MODEL_MESH_DATA`` entries, and the same order
    ``CModel::m_Meshes`` is built in (one ``CMesh`` per submesh, each with its own vertex
    buffer indexed from 0). A flat vertex index is only meaningful inside this tool; the
    runtime addresses vertices as (mesh index, local index) -- see global_to_local()."""
    data = path.read_bytes()
    magic, _major, _minor, _flags, _content_size = WMODEL_FILE_HEADER.unpack_from(data, 0)
    if magic != b"WINT":
        raise SystemExit("%s: not a WINT container" % path)
    base = WMODEL_FILE_HEADER.size
    _magic, section_count, _anim_count, _model_flags = WMODEL_MODEL_HEADER.unpack_from(data, base)[:4]
    descs = [WMODEL_SECTION_DESC.unpack_from(data, base + WMODEL_MODEL_HEADER.size + i * WMODEL_SECTION_DESC.size)
             for i in range(section_count)]

    for section_type, _index, offset, _size, name in descs:
        if section_type != WMODEL_SECTION_MESH:
            continue
        for candidate in (base + offset, offset):
            if data[candidate:candidate + 4] == b"WINT":
                start = candidate
                break
        else:
            raise SystemExit("%s: mesh section not found at %d" % (path, offset))
        payload = start + WMODEL_FILE_HEADER.size
        (mesh_magic, submesh_count, _bone_count, _vflags, stride, vertex_count,
         *_rest) = WMODEL_MESH_HEADER.unpack_from(data, payload)
        if mesh_magic != b"WMSH":
            raise SystemExit("%s: expected WMSH, got %r" % (path, mesh_magic))
        submesh_base = payload + WMODEL_MESH_HEADER.size
        submeshes = []
        for i in range(submesh_count):
            vertex_offset_bytes, vertex_count_i, _index_offset, _index_count, material_index, _material_hash, _name = \
                WMODEL_SUBMESH_DESC.unpack_from(data, submesh_base + i * WMODEL_SUBMESH_DESC.size)
            # SUBMESH_DESC.vertexOffset is a BYTE offset into the flat vertex blob
            # (WMeshReader.cpp: "pVertexBlob + sourceMesh.vertexOffset"), not a vertex index.
            if vertex_offset_bytes % stride != 0:
                raise SystemExit("%s: submesh %d vertexOffset %d is not a multiple of stride %d"
                                 % (path, i, vertex_offset_bytes, stride))
            submeshes.append((vertex_offset_bytes // stride, vertex_count_i, material_index))
        vbase = submesh_base + submesh_count * WMODEL_SUBMESH_DESC.size
        vertices = []
        for i in range(vertex_count):
            record = vbase + i * stride
            position = struct.unpack_from("<3f", data, record)
            uv = struct.unpack_from("<2f", data, record + 24)
            vertices.append((position, uv))
        return vertices, submeshes
    raise SystemExit("%s: no mesh section found" % path)


def global_to_local(global_index: int, submeshes):
    """Convert a flat vertex index into (meshIndex, localIndex) -- the addressing the
    runtime .wmodel actually uses (one CMesh per submesh, each vertex-buffer-local)."""
    for mesh_index, (vertex_offset, vertex_count, _material_index) in enumerate(submeshes):
        if vertex_offset <= global_index < vertex_offset + vertex_count:
            return mesh_index, global_index - vertex_offset
    raise SystemExit("global vertex index %d falls outside every submesh's range" % global_index)


# --------------------------------------------------------------------- mapping

def find_axis_transform(psk_points, wmodel_positions, sample_size=150):
    """Brute-force the axis permutation/sign/scale that makes the two point
    clouds coincide. LostArk .wmodel cooks have so far always turned out to be
    an identity permutation with one sign flip at scale 1, but this checks
    rather than assumes it, so a different class or a future cook convention
    is caught instead of silently mismatched.

    Acceptance is by *median* per-sample squared distance, not the sum: a
    dedicated face .wmodel (one class) lines up bit-exact on every sample, but
    a combined full-body .wmodel (the other three) re-bakes the face at a
    very slightly different bind pose, so a handful of samples land a
    fraction of a millimetre off even under the right transform. The median
    is insensitive to that minority; the sum was not, and rejected an
    otherwise-correct transform outright."""
    import itertools
    import random

    random.seed(0)
    sample = random.sample(psk_points, min(sample_size, len(psk_points)))
    best = None
    for perm in itertools.permutations(range(3)):
        # A cook re-expresses the mesh in another right-handed basis, so only the 24 proper
        # rotations are candidates; the other 24 are reflections. Both score near zero here --
        # a face is roughly symmetric, so a mirrored wedge still lands very close to some real
        # vertex -- and Artist actually picked a reflection, which paired every wedge with its
        # mirror-image vertex. That put b_fc_l_* weights on the model's right and left the 14%
        # of the face where the mesh is not symmetric with no exact match at all. Rejecting
        # reflections outright is what separates the two; a distance threshold cannot.
        parity = 1 if (perm[1] - perm[0]) * (perm[2] - perm[0]) * (perm[2] - perm[1]) > 0 else -1
        for signs in itertools.product((1, -1), repeat=3):
            if parity * signs[0] * signs[1] * signs[2] < 0:
                continue
            for scale in (0.01, 1.0, 100.0):
                distances = []
                for p in sample:
                    q = (p[perm[0]] * signs[0] * scale,
                         p[perm[1]] * signs[1] * scale,
                         p[perm[2]] * signs[2] * scale)
                    distances.append(min((q[0] - c[0]) ** 2 + (q[1] - c[1]) ** 2 + (q[2] - c[2]) ** 2
                                         for c in wmodel_positions))
                distances.sort()
                median = distances[len(distances) // 2]
                if best is None or median < best[0]:
                    best = (median, perm, signs, scale)
    median, perm, signs, scale = best
    if median > 1e-6:
        raise SystemExit("no axis/sign/scale combination lines the meshes up "
                         "(best median squared distance %.6f over %d samples)" % (median, len(sample)))
    return lambda p: (p[perm[0]] * signs[0] * scale,
                       p[perm[1]] * signs[1] * scale,
                       p[perm[2]] * signs[2] * scale)


def build_mapping(psk_points, wedges, wmodel_vertices, require_full_coverage=True):
    transform = find_axis_transform(psk_points, [pos for pos, _uv in wmodel_vertices])

    # Cell size drives the neighbour search below (a 3x3x3 block of cells), so it must be
    # at least as large as the widest fallback radius tried in nearby_matches().
    cell = 1.0
    grid = defaultdict(list)
    for i, (position, _uv) in enumerate(wmodel_vertices):
        grid[tuple(round(c / cell) for c in position)].append(i)

    def nearby_matches(q, epsilon_sq):
        key = tuple(round(c / cell) for c in q)
        out = []
        for dx in (-1, 0, 1):
            for dy in (-1, 0, 1):
                for dz in (-1, 0, 1):
                    for i in grid.get((key[0] + dx, key[1] + dy, key[2] + dz), ()):
                        position, _uv = wmodel_vertices[i]
                        d2 = sum((a - b) ** 2 for a, b in zip(position, q))
                        if d2 < epsilon_sq:
                            out.append(i)
        return out

    drifted = 0
    mapping = []
    for point_index, u, v, _material in wedges:
        q = transform(psk_points[point_index])
        candidates = nearby_matches(q, EXACT_EPSILON_SQ)
        if not candidates:
            # A combined-body .wmodel bakes a handful of vertices a fraction of a
            # millimetre off the standalone retail face mesh (see find_axis_transform);
            # widen the search only for those, and only as far as it takes to find the
            # single nearest vertex -- never silently take a distant "close enough" one.
            for epsilon_sq in (1e-4, 1e-2, cell * cell):
                candidates = nearby_matches(q, epsilon_sq)
                if candidates:
                    break
            if not candidates:
                raise SystemExit("wedge at %r has no runtime vertex within %.3g of that position"
                                 % (q, cell))
            best = min(candidates, key=lambda i: sum((a - b) ** 2 for a, b in
                                                      zip(wmodel_vertices[i][0], q)))
            candidates = [best]
            drifted += 1
        if len(candidates) == 1:
            mapping.append(candidates)
            continue
        scored = []
        for i in candidates:
            _position, (cu, cv) = wmodel_vertices[i]
            distance = min((cu - u) ** 2 + (cv - v) ** 2, (cu - u) ** 2 + (cv - (1.0 - v)) ** 2)
            scored.append((distance, i))
        scored.sort()
        tied = [i for distance, i in scored if distance < UV_TIE_EPSILON_SQ] or [scored[0][1]]
        mapping.append(tied)

    covered = {i for targets in mapping for i in targets}
    if require_full_coverage and len(covered) != len(wmodel_vertices):
        missing = len(wmodel_vertices) - len(covered)
        raise SystemExit("%d runtime vertices are not claimed by any source wedge" % missing)
    return mapping, len(covered), drifted


# --------------------------------------------------------------------- writing

def write_facemorphmap(path: Path, submeshes, mapping) -> None:
    """``mapping`` entries are lists of (meshIndex, localIndex) pairs -- CModel::m_Meshes
    addressing, not a flat wmodel-wide index (see global_to_local())."""
    mesh_vertex_counts = [vertex_count for _vertex_offset, vertex_count, _material_index in submeshes]
    body = bytearray()
    for targets in mapping:
        body += struct.pack("<I", len(targets))
        for mesh_index, local_index in targets:
            body += struct.pack("<2I", mesh_index, local_index)
    header = struct.pack("<8s3I", FACEMORPHMAP_MAGIC, FACEMORPHMAP_VERSION,
                         len(mapping), len(mesh_vertex_counts))
    header += struct.pack("<%dI" % len(mesh_vertex_counts), *mesh_vertex_counts)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(header + bytes(body))


def main() -> int:
    repo = Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser()
    parser.add_argument("--psk", type=Path,
                        help="a real exported PSK (mutually exclusive with --upk)")
    parser.add_argument("--upk", type=Path,
                        help="retail .upk to read the vertex buffer directly from "
                             "(mutually exclusive with --psk)")
    parser.add_argument("--object-name", help="skeletalmesh export name, required with --upk")
    parser.add_argument("--expect-verts", type=int,
                        help="known NumVertices (e.g. a .facemorphs vertexIndexBound), required with --upk")
    parser.add_argument("--wmodel", type=Path, required=True)
    parser.add_argument("--mesh-name", required=True,
                        help="wmodel mesh section name (the retail SkeletalMesh export name)")
    parser.add_argument("--class-id", required=True)
    parser.add_argument("--resources", type=Path, default=repo / "Client" / "Bin" / "Resources")
    args = parser.parse_args()

    if bool(args.psk) == bool(args.upk):
        raise SystemExit("pass exactly one of --psk or --upk")
    if args.psk:
        psk_points, wedges = read_psk(args.psk)
    else:
        if not (args.object_name and args.expect_verts):
            raise SystemExit("--upk requires --object-name and --expect-verts")
        verts = read_skeletalmesh_vertices.read_vertices(args.upk, args.object_name, args.expect_verts)
        psk_points = [position for position, _uv in verts]
        wedges = [(i, uv[0], uv[1], 0) for i, (_position, uv) in enumerate(verts)]

    wmodel_vertices, submeshes = read_wmodel_mesh(args.wmodel, args.mesh_name)
    # A dedicated face .wmodel (LanceMaster) is entirely face -- every one of its vertices
    # must be claimed. A combined full-body .wmodel (the other three classes: no dedicated
    # face .wmodel exists for them) carries body/clothing vertices no face wedge will ever
    # reach, so partial coverage there is expected, not an error.
    combined_body = len(wmodel_vertices) > len(wedges) * 2
    mapping, covered, drifted = build_mapping(psk_points, wedges, wmodel_vertices,
                                              require_full_coverage=not combined_body)
    local_mapping = [[global_to_local(i, submeshes) for i in targets] for targets in mapping]

    out = args.resources / "Character" / args.class_id / "FaceMorphs" / (args.class_id + ".facemorphmap")
    write_facemorphmap(out, submeshes, local_mapping)

    single = sum(1 for targets in mapping if len(targets) == 1)
    print("%s: %d source wedges (%d needed a widened search), %d runtime vertices (%s, %d claimed), "
          "%d wedges map 1:1, %d map to more than one runtime vertex, %d bytes -> %s"
          % (args.class_id, len(wedges), drifted, len(wmodel_vertices),
             "combined body" if combined_body else "dedicated face mesh", covered, single,
             len(mapping) - single, out.stat().st_size, out))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
