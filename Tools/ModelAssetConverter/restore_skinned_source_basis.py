#!/usr/bin/env python3
"""Restore a legacy ActorX WModel's smooth basis from its exact UModel glTF.

The supported conversion is glTF (x,y,z) -> legacy WModel (x,-z,-y),
with unchanged UV0 and reversed triangle winding. Every indexed corner must
match before writing. This is not a normal smoothing or mesh subdivision tool.
Materials, skeleton, clips, weights and triangle positions remain unchanged.
WMSH 1.5 adds the source tangent sign to the existing 76-byte vertex record.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
import struct
from pathlib import Path

import cook_wmodel_geometry_contract as wm


def unit(values):
    wm.require(all(math.isfinite(v) for v in values), "non-finite source basis")
    length = math.sqrt(sum(v * v for v in values))
    wm.require(length > 1e-8, "zero source basis")
    return tuple(v / length for v in values)


def basis_vector(values):
    return unit((values[0], -values[2], -values[1]))


def read_source(path: Path):
    document = json.loads(path.read_text(encoding="utf-8"))
    wm.require(len(document["meshes"]) == 1, "select exactly one source mesh/LOD")
    buffers = []
    for item in document["buffers"]:
        uri = item["uri"]
        resolved = (path.parent / uri).resolve()
        wm.require(resolved.is_relative_to(path.parent.resolve()), "source buffer leaves glTF directory")
        blob = resolved.read_bytes()
        wm.require(len(blob) == item["byteLength"], "source buffer size differs")
        buffers.append(blob)

    def accessor(index):
        value = document["accessors"][index]
        wm.require("sparse" not in value, "sparse source accessor is unsupported")
        view = document["bufferViews"][value["bufferView"]]
        components = {"SCALAR": 1, "VEC2": 2, "VEC3": 3, "VEC4": 4}[value["type"]]
        kind = {5121: "B", 5123: "H", 5125: "I", 5126: "f"}[value["componentType"]]
        record = struct.Struct("<" + kind * components)
        stride = view.get("byteStride", record.size)
        relative = value.get("byteOffset", 0)
        count = value["count"]
        wm.require(count > 0 and stride >= record.size and relative >= 0
                   and relative + (count - 1) * stride + record.size <= view["byteLength"],
                   "source accessor exceeds buffer view")
        start = view.get("byteOffset", 0) + relative
        blob = buffers[view["buffer"]]
        result = [record.unpack_from(blob, start + i * stride) for i in range(count)]
        wm.require(all(math.isfinite(v) for row in result for v in row), "non-finite source accessor")
        return result

    corners = []
    for primitive in document["meshes"][0]["primitives"]:
        wm.require(primitive.get("mode", 4) == 4, "source primitive is not triangles")
        attributes = primitive["attributes"]
        positions, normals, tangents, uvs = [accessor(attributes[k])
            for k in ("POSITION", "NORMAL", "TANGENT", "TEXCOORD_0")]
        wm.require(len({len(positions), len(normals), len(tangents), len(uvs)}) == 1,
                   "source channel counts disagree")
        indices = [row[0] for row in accessor(primitive["indices"])]
        wm.require(len(indices) % 3 == 0 and all(0 <= i < len(positions) for i in indices),
                   "invalid source index range")
        for start in range(0, len(indices), 3):
            for i in (indices[start + 2], indices[start + 1], indices[start]):
                p, n, t, uv = positions[i], normals[i], tangents[i], uvs[i]
                wm.require(t[3] in (-1.0, 1.0), "invalid source tangent sign")
                corners.append(((p[0], -p[2], -p[1]), uv,
                                basis_vector(n), basis_vector(t), -t[3]))
    return corners, [hashlib.sha256(blob).hexdigest() for blob in buffers]


def restore(original: bytes, corners):
    parsed = wm.parse_skinned_uv_wmodel(original)
    wm.require(parsed["versionMinor"] == 0, "restore requires the unmodified legacy 1.0 model")
    mesh = parsed["mesh"]
    header = list(parsed["meshHeader"])
    wm.require(len(corners) == header[6], "source and installed triangle counts differ")
    vertex_start, index_start = parsed["vertexStart"], parsed["indexStart"]
    index_type = "H" if header[7] == 2 else "I"
    vertex_blocks, index_blocks, descriptors, summaries = [], [], [], []
    cursor = vertex_offset = index_offset = 0
    max_position_error = max_uv_error = 0.0
    for submesh in parsed["submeshes"]:
        indices = struct.unpack_from("<" + index_type * submesh[3], mesh, index_start + submesh[2])
        vertices = [mesh[vertex_start + submesh[0] + i * 76:
                         vertex_start + submesh[0] + (i + 1) * 76] for i in range(submesh[1])]
        # Keep every original vertex/index slot unless a shared vertex really
        # has multiple source bases. Only that explicit split appends a slot.
        rebuilt, remap, output_indices = [None] * len(vertices), {}, []
        positive = negative = 0
        visited = set()
        for old_index in indices:
            wm.require(old_index < len(vertices), "invalid installed index")
            old = vertices[old_index]
            position = struct.unpack_from("<3f", old)
            uv = struct.unpack_from("<2f", old, 24)
            source_position, source_uv, normal, tangent, sign = corners[cursor]
            cursor += 1
            position_error = max(abs(a - b) for a, b in zip(position, source_position))
            uv_error = max(abs(a - b) for a, b in zip(uv, source_uv))
            wm.require(position_error <= 1e-5 and uv_error <= 1e-6,
                       f"source triangle corner {cursor - 1} differs: position={position_error}, uv={uv_error}")
            max_position_error = max(max_position_error, position_error)
            max_uv_error = max(max_uv_error, uv_error)
            basis = struct.pack("<7f", *normal, *tangent, sign)
            key = (old_index, basis)
            if key not in remap:
                output = bytearray(old)
                struct.pack_into("<3f", output, 12, *normal)
                struct.pack_into("<3f", output, 32, *tangent)
                output.extend(struct.pack("<f", sign))
                wm.require(output[:12] == old[:12] and output[24:32] == old[24:32]
                           and output[44:76] == old[44:76], "position/UV/skin payload changed")
                if rebuilt[old_index] is None:
                    remap[key] = old_index
                    rebuilt[old_index] = bytes(output)
                else:
                    remap[key] = len(rebuilt)
                    rebuilt.append(bytes(output))
                positive += sign > 0
                negative += sign < 0
            output_indices.append(remap[key])
            visited.add(old_index)
        wm.require(len(visited) == len(vertices), "unreferenced installed vertices have no source basis")
        wm.require(not output_indices or max(output_indices) < (1 << (header[7] * 8)),
                   "restored vertex count exceeds current index format")
        desc = list(submesh)
        desc[0], desc[1], desc[2] = vertex_offset, len(rebuilt), index_offset
        descriptors.append(wm.SUBMESH_DESC.pack(*desc))
        vertex_block = b"".join(rebuilt)
        index_block = struct.pack("<" + index_type * len(output_indices), *output_indices)
        vertex_blocks.append(vertex_block)
        index_blocks.append(index_block)
        vertex_offset += len(vertex_block)
        index_offset += len(index_block)
        summaries.append({"material": submesh[4], "verticesBefore": submesh[1],
                          "verticesAfter": len(rebuilt), "triangles": submesh[3] // 3,
                          "positiveHandedness": positive, "negativeHandedness": negative})
    header[3] |= wm.VF_TANGENT_HANDEDNESS
    header[4] = wm.STRIDE_SKINNED_BASIS
    header[5] = vertex_offset // header[4]
    tail = mesh[index_start + header[6] * header[7]:]
    content = (wm.MESH_HEADER.pack(*header) + b"".join(descriptors) + b"".join(vertex_blocks)
               + b"".join(index_blocks) + tail)
    new_mesh = wm.FILE_HEADER.pack(b"WINT", 1, 5, 0, len(content)) + content
    result = wm.rebuild_wmodel(parsed["modelHeader"], parsed["sections"], new_mesh)
    readback = wm.parse_skinned_uv_wmodel(result)
    wm.require([s for s in readback["sections"] if s.type_id != 1]
               == [s for s in parsed["sections"] if s.type_id != 1],
               "materials, skeleton or animation section changed")
    return result, {"triangles": cursor // 3, "maxPositionMatchError": max_position_error,
                    "maxUVMatchError": max_uv_error, "submeshes": summaries,
                    "nonMeshSectionsByteIdentical": True, "formatVersion": "1.5"}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--model", type=Path, required=True)
    parser.add_argument("--source-gltf", type=Path, required=True)
    parser.add_argument("--source-package", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--report", type=Path, required=True)
    args = parser.parse_args()
    wm.require(args.model.resolve() != args.output.resolve(), "write a candidate; in-place replacement is forbidden")
    wm.require(not args.output.exists(), "candidate already exists")
    original = args.model.read_bytes()
    corners, buffer_hashes = read_source(args.source_gltf)
    result, report = restore(original, corners)
    report.update({"input": str(args.model.resolve()), "output": str(args.output.resolve()),
                   "inputSha256": hashlib.sha256(original).hexdigest(),
                   "outputSha256": hashlib.sha256(result).hexdigest(),
                   "sourceGltf": str(args.source_gltf.resolve()),
                   "sourceGltfSha256": hashlib.sha256(args.source_gltf.read_bytes()).hexdigest(),
                   "sourceBufferSha256": buffer_hashes,
                   "sourcePackage": str(args.source_package.resolve()),
                   "sourcePackageSha256": hashlib.sha256(args.source_package.read_bytes()).hexdigest()})
    wm.require(args.model.read_bytes() == original, "installed model changed while preparing candidate")
    wm.write_atomic(args.output, result)
    wm.write_atomic(args.report, (json.dumps(report, indent=2) + "\n").encode("utf-8"))
    print(json.dumps(report))


if __name__ == "__main__":
    main()
