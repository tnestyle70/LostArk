"""Join a vehicle PSK's EXTRAUVS0 set onto its NPC-pipeline 1.0 skinned WModel as UV1.

The NPC pipeline drops extra UV sets, while materials such as the Serene Starlight
Blessing shell (program 88) sample TexCoord[1]. Cooked vertices are split and reordered,
so each one is joined through its triangles: a cooked triangle matches the PSK face with
the same corner positions (PSK -> cooked transform fitted from unambiguous UV0 anchors)
and the same UV0, and every corner takes that face wedge's UV1. The write refuses unless
every triangle matches and no vertex receives two different UV1 values.

usage:
  python cook_psk_extra_uv1.py --wmodel <Model.wmodel> --source-psk <mesh_sk.psk> [--dry-run]
"""
from __future__ import annotations

import argparse
import collections
import json
import struct
import sys
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "ModelAssetConverter"))
sys.path.insert(0, str(Path(__file__).resolve().parent))
import cook_wmodel_geometry_contract as cook  # noqa: E402
import cook_single_set_uv1 as single  # noqa: E402

COOKED_TEXCOORD_OFFSET = 24


def read_psk(path: Path):
    data = path.read_bytes()
    chunks, offset = {}, 0
    while offset + 32 <= len(data):
        name = data[offset:offset + 20].split(b"\0")[0].decode()
        size, count = struct.unpack_from("<ii", data, offset + 24)
        chunks[name] = (offset + 32, size, count)
        offset += 32 + size * count
    for required in ("PNTS0000", "VTXW0000", "FACE0000", "MATT0000", "EXTRAUVS0"):
        if required not in chunks:
            raise SystemExit("%s: missing %s chunk" % (path.name, required))

    def rows(name, fmt):
        at, size, count = chunks[name]
        return [struct.unpack_from(fmt, data, at + i * size) for i in range(count)]

    at, size, count = chunks["MATT0000"]
    materials = [data[at + i * size:at + i * size + 64].split(b"\0")[0].decode().lower() for i in range(count)]
    return (np.array(rows("PNTS0000", "<3f")), rows("VTXW0000", "<iffB"), rows("FACE0000", "<HHH"),
            materials, rows("EXTRAUVS0", "<2f"))


def uv_key(u, v):
    return (round(u, 5), round(v, 5))


def position_key(p):
    return tuple(int(round(c * 1000)) for c in p)


def join_submesh(parsed, index, material, points, wedges, faces, extra):
    desc = parsed["submeshes"][index]
    stride, istride = parsed["meshHeader"][4], parsed["meshHeader"][7]
    base = parsed["vertexStart"] + desc[0]
    count = desc[1]
    mesh = parsed["mesh"]
    pos = np.array([struct.unpack_from("<3f", mesh, base + i * stride) for i in range(count)])
    uv0 = [struct.unpack_from("<2f", mesh, base + i * stride + COOKED_TEXCOORD_OFFSET) for i in range(count)]

    by_uv = collections.defaultdict(list)
    for wi, wedge in enumerate(wedges):
        if wedge[3] == material:
            by_uv[uv_key(wedge[1], wedge[2])].append(wi)
    anchors_src, anchors_dst = [], []
    for i in range(count):
        found = by_uv[uv_key(*uv0[i])]
        if len({wedges[w][0] for w in found}) == 1:
            anchors_src.append(points[wedges[found[0]][0]])
            anchors_dst.append(pos[i])
    if len(anchors_src) < 4:
        raise SystemExit("submesh %d: too few unambiguous UV0 anchors (%d)" % (index, len(anchors_src)))
    design = np.hstack([np.array(anchors_src), np.ones((len(anchors_src), 1))])
    transform, *_ = np.linalg.lstsq(design, np.array(anchors_dst), rcond=None)
    fit_error = float(np.abs(design @ transform - np.array(anchors_dst)).max())
    placed = np.hstack([points, np.ones((len(points), 1))]) @ transform

    face_table = collections.defaultdict(list)
    for corners in faces:
        if wedges[corners[0]][3] == material:
            face_table[frozenset(position_key(placed[wedges[c][0]]) for c in corners)].append(corners)

    fmt = "<H" if istride == 2 else "<I"
    first_index = desc[2] // istride
    votes = [collections.Counter() for _ in range(count)]
    unmatched = 0
    for t in range(0, desc[3], 3):
        tri = [struct.unpack_from(fmt, mesh, parsed["indexStart"] + (first_index + t + c) * istride)[0]
               for c in range(3)]
        chosen = None
        for corners in face_table.get(frozenset(position_key(pos[v]) for v in tri), []):
            picked = []
            for v in tri:
                match = [c for c in corners if position_key(placed[wedges[c][0]]) == position_key(pos[v])
                         and uv_key(wedges[c][1], wedges[c][2]) == uv_key(*uv0[v])]
                picked.append(match[0] if len(match) == 1 else None)
            if all(p is not None for p in picked):
                chosen = picked
                break
        if chosen is None:
            unmatched += 1
            continue
        for wedge, v in zip(chosen, tri):
            votes[v][uv_key(*extra[wedge])] += 1

    ambiguous = sum(len(v) > 1 for v in votes)
    missing = sum(not v for v in votes)
    uv1 = [tuple(float(c) for c in v.most_common(1)[0][0]) if v else (0.0, 0.0) for v in votes]
    report = dict(submesh=index, vertices=count, triangles=desc[3] // 3, anchors=len(anchors_src),
                  fitMaxError=fit_error, unmatchedTriangles=unmatched, ambiguousVertices=ambiguous,
                  missingVertices=missing)
    return uv1, report


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--wmodel", type=Path, required=True)
    parser.add_argument("--source-psk", type=Path, required=True)
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    data = args.wmodel.read_bytes()
    parsed = cook.parse_skinned_uv_wmodel(data)
    if parsed["versionMinor"] != 0:
        raise SystemExit("%s: already 1.%d; cook from the legacy 1.0 file" % (args.wmodel.name, parsed["versionMinor"]))
    names = single.read_material_names(data)
    submesh_materials = single.read_submesh_materials(data)
    points, wedges, faces, materials, extra = read_psk(args.source_psk)

    channels, reports = {}, []
    for index, material in enumerate(submesh_materials):
        name = names.get(material)
        if name not in materials:
            raise SystemExit("submesh %d material %r is not in %s" % (index, name, args.source_psk.name))
        uv1, report = join_submesh(parsed, index, materials.index(name), points, wedges, faces, extra)
        report["material"] = name
        reports.append(report)
        channels[index] = {"TEXCOORD_1": uv1}
    print(json.dumps(reports, indent=1))
    if any(r["unmatchedTriangles"] or r["ambiguousVertices"] or r["missingVertices"] for r in reports):
        raise SystemExit("join is incomplete; nothing written")

    written, receipt = cook.cook_skinned_uv_contract(data, channels)
    print(json.dumps(receipt, sort_keys=True)[:240])
    if args.dry_run:
        print("dry run, nothing written")
        return 0
    args.wmodel.write_bytes(written)
    print("written, %d bytes" % len(written))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
