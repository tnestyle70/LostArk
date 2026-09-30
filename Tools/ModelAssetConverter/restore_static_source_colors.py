#!/usr/bin/env python3
"""Restore static WModel COLOR0 from verified UE3 native color streams.

UModel's optional glTF COLOR_0 is not evidence that a cooked UE3 mesh owns
colors. For KR version 868/16, identify the adjacent position, UV/basis and
FStaticMeshColorStream3 arrays and join every glTF vertex against all preserved
native channels. An empty native stream removes COLOR_0. A nonempty stream is
mapped from serialized BGRA to RGBA without filtering. Existing WModel vertex
order, all other channels, indices, bounds and material section stay byte exact.
Only out-of-place candidates are accepted. No runtime or authoring installation.
"""
from __future__ import annotations

import argparse
import collections
import copy
import hashlib
import json
from pathlib import Path
import shutil
import struct
import sys

import numpy as np
from scipy.spatial import cKDTree

import cook_wmodel_geometry_contract as geometry

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'Tools/LevelPlacementExtractor'))
import extract_ue3_texture_mips as packages  # noqa: E402
from source_extraction_io import write_atomic, write_pair  # noqa: E402


def digest(data):
    return hashlib.sha256(data).hexdigest()


def read_json(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def save_json(path, value):
    write_atomic(path, (json.dumps(value, ensure_ascii=False, indent=2) + '\n').encode('utf-8'))


def decode_gltf(path):
    doc = read_json(path)
    arrays, cache = [], {}
    for mesh in doc['meshes']:
        for primitive in mesh['primitives']:
            arrays.append({name: np.array(geometry.accessor_values(doc, path.parent, index, cache)[0])
                           for name, index in primitive['attributes'].items()})
    geometry.require(arrays and all(all(k in a for k in ('POSITION', 'NORMAL', 'TANGENT', 'TEXCOORD_0')) for a in arrays),
                     'Missing source glTF geometry channels')
    return doc, arrays


def native_stream_candidates(raw, max_count):
    """Find complete consecutive buffers; never infer absence from missing color data."""
    found = []
    for offset in range(len(raw) - 32):
        uv, stride, count, fp, bulk_stride, bulk_count = struct.unpack_from('<6I', raw, offset)
        if not (1 <= uv <= 4 and fp in (0, 1) and 0 < count <= max_count
                and stride == 8 + uv * (8 if fp else 4) and stride == bulk_stride and count == bulk_count):
            continue
        position_header = offset - 16 - count * 12
        if position_header < 0 or struct.unpack_from('<4I', raw, position_header) != (12, count, 12, count):
            continue
        color_header = offset + 24 + count * stride
        if color_header + 8 > len(raw):
            continue
        color_stride, color_count = struct.unpack_from('<2I', raw, color_header)
        if (color_stride, color_count) == (0, 0):
            color_offset = None
            next_offset = color_header + 8
        elif (color_stride, color_count) == (4, count) and color_header + 16 + count * 4 <= len(raw):
            if struct.unpack_from('<2I', raw, color_header + 8) != (4, count):
                continue
            color_offset = color_header + 16
            next_offset = color_offset + count * 4
        else:
            continue
        # The LOD vertex count immediately follows the optional color bulk.
        if next_offset + 4 > len(raw) or struct.unpack_from('<I', raw, next_offset)[0] != count:
            continue
        found.append(dict(positionHeader=position_header, uvHeader=offset, colorHeader=color_header,
                          colorOffset=color_offset, count=count, uvCount=uv, uvStride=stride, floatUv=bool(fp)))
    return found


def join_native_colors(raw, arrays):
    candidates = native_stream_candidates(raw, sum(len(a['POSITION']) for a in arrays))
    accepted, rejected = [], []
    for stream in candidates:
        n, uv_count, stride = stream['count'], stream['uvCount'], stream['uvStride']
        position = np.frombuffer(raw, dtype='<f4', count=n * 3, offset=stream['positionHeader'] + 16).reshape(n, 3)
        # UModel swaps UE Y/Z and converts cm to meters before glTF export.
        positions = position[:, [0, 2, 1]] * np.float32(.01)
        at = stream['uvHeader'] + 24
        packed = np.array([list(raw[at + i * stride:at + i * stride + 8]) for i in range(n)])
        # KR UModel's signed packed axes use byte-128. The common scale cancels
        # during normalization; using /127.5-1 incorrectly shifts neutral 128.
        tangent = (packed[:, :3] - 128.0)[:, [0, 2, 1]]
        normal = (packed[:, 4:7] - 128.0)[:, [0, 2, 1]]
        tangent /= np.linalg.norm(tangent, axis=1)[:, None]
        normal /= np.linalg.norm(normal, axis=1)[:, None]
        uv_format = '<' + ('f' if stream['floatUv'] else 'e') * uv_count * 2
        uvs = np.array([struct.unpack_from(uv_format, raw, at + i * stride + 8) for i in range(n)])
        key = np.concatenate((positions, uvs), axis=1)
        tree = cKDTree(key)
        colors = None if stream['colorOffset'] is None else np.frombuffer(raw, np.uint8, count=n * 4, offset=stream['colorOffset']).reshape(n, 4)[:, [2, 1, 0, 3]]
        joined, used, maximum = [], set(), 0.0
        try:
            for data in arrays:
                geometry.require(all('TEXCOORD_' + str(i) in data for i in range(uv_count)), 'Source glTF omits a native UV channel')
                source_key = np.concatenate([data['POSITION']] + [data['TEXCOORD_' + str(i)] for i in range(uv_count)], axis=1)
                result = []
                for index, values in enumerate(source_key):
                    matches = [j for j in tree.query_ball_point(values, 1e-4)
                               if np.max(np.abs(tangent[j] - data['TANGENT'][index, :3])) < 1e-6
                               and np.max(np.abs(normal[j] - data['NORMAL'][index])) < 1e-6]
                    geometry.require(matches, f'No native position/UV/basis match for glTF vertex {index}')
                    mapped = {None if colors is None else bytes(colors[j]) for j in matches}
                    geometry.require(len(mapped) == 1, f'Ambiguous native color at glTF vertex {index}')
                    maximum = max(maximum, min(float(np.max(np.abs(key[j] - values))) for j in matches))
                    used.update(matches)
                    result.append(next(iter(mapped)))
                joined.append(result)
            # A vertex can appear in several material sections; native count need not equal
            # the sum of glTF primitive counts. Every native vertex must still be covered.
            geometry.require(len(used) == n, 'Native vertex stream is not fully covered by source glTF')
            accepted.append((stream, joined, maximum))
        except ValueError as error:
            rejected.append(str(error))
            continue
    geometry.require(len(accepted) == 1, f'Native color stream join missing or ambiguous: headers={len(candidates)}, matches={len(accepted)}, rejected={rejected}')
    stream, colors, maximum = accepted[0]
    return colors, {**stream, 'nativeSerialSHA256': digest(raw), 'hasNativeColors': stream['colorOffset'] is not None,
                    'completePositionUvBasisJoin': True, 'maxPositionUvJoinError': maximum,
                    'gltfVertexCount': sum(len(a['POSITION']) for a in arrays), 'nativeVertexCount': stream['count']}


def stage_source(source_receipt, directory):
    source = read_json(source_receipt)
    gltf = source_receipt.parent / source['gltf']
    geometry.require(not directory.resolve().is_relative_to(source_receipt.parent.resolve())
                     and not source_receipt.parent.resolve().is_relative_to(directory.resolve()),
                     'Staging directory must be separate from the source export')
    gltf_before = gltf.read_bytes()
    package_path = Path(source['physicalPackagePath'])
    package = packages.SourcePackage(package_path)
    geometry.require((package.summary.version, package.summary.licensee_version) == (868, 16), 'Only verified KR 868/16 static serialization is supported')
    index = package.find(source['fullPath'].split('.', 1)[1])
    geometry.require(package.cls(index) == 'staticmesh', 'Source object is not StaticMesh')
    raw = package.raw(index)
    doc, arrays = decode_gltf(gltf)
    colors, proof = join_native_colors(raw, arrays)
    # Reuse the existing native parallel N/T proof; never regenerate a basis.
    if 'nativeParallelBasisProof' in source:
        sys.path.insert(0, str(ROOT / 'Tools/CharacterSelectPipeline'))
        import prove_native_static_parallel_basis as parallel
        verified, _ = parallel.verify_and_stage(source_receipt, gltf, directory / 'parallel-proof')
        doc = read_json(verified)
    primitives = [p for m in doc['meshes'] for p in m['primitives']]
    corrected = copy.deepcopy(doc)
    corrected_primitives = [p for m in corrected['meshes'] for p in m['primitives']]
    directory.mkdir(parents=True, exist_ok=True)
    for buffer in corrected['buffers']:
        uri = Path(buffer['uri'])
        geometry.require(not uri.is_absolute() and '..' not in uri.parts, 'glTF buffer escapes source directory')
        destination = directory / uri
        destination.parent.mkdir(parents=True, exist_ok=True)
        write_atomic(destination, (gltf.parent / uri).read_bytes())
    color_blob = bytearray()
    for primitive, values in zip(corrected_primitives, colors):
        primitive['attributes'].pop('COLOR_0', None)
        if proof['hasNativeColors']:
            offset = len(color_blob)
            color_blob.extend(b''.join(values))
            view = len(corrected['bufferViews'])
            corrected['bufferViews'].append(dict(buffer=len(corrected['buffers']), byteOffset=offset, byteLength=len(values) * 4))
            accessor = len(corrected['accessors'])
            corrected['accessors'].append(dict(bufferView=view, componentType=5121, count=len(values), type='VEC4', normalized=True))
            primitive['attributes']['COLOR_0'] = accessor
    if color_blob:
        geometry.require(all(b['uri'] != 'native-colors.bin' for b in corrected['buffers']), 'native color buffer name collision')
        corrected['buffers'].append(dict(uri='native-colors.bin', byteLength=len(color_blob)))
        write_atomic(directory / 'native-colors.bin', bytes(color_blob))
    output = directory / gltf.name
    save_json(output, corrected)
    geometry.parse_source_gltf(output)
    geometry.require(package_path.read_bytes() == package.physical and gltf.read_bytes() == gltf_before, 'Source package or glTF changed')
    proof.update(sourceObject=source['fullPath'], sourcePackage=str(package_path), sourcePackageSHA256=package.digest,
                 sourceGltf=str(gltf), sourceGltfSHA256=digest(gltf.read_bytes()), correctedGltf=str(output),
                 originalBuffersByteIdentical=True, nativeColorChannelOrder='BGRA to RGBA',
                 sourceColorRule='Preserve empty native stream as absent; runtime owns absent-channel white default',
                 nativeParallelBasisProofReused='nativeParallelBasisProof' in source)
    save_json(directory / 'native-colors.receipt.json', proof)
    return output, proof


def vertex_key(values, uv1=None, uv2=None):
    # Normalize signed zero for matching only; preserve original WModel bytes in output.
    # Tangent handedness is not a color identity. Older exports can carry a
    # different W despite identical native packed XYZ, position and UV channels.
    # Preserve the installed W verbatim; this tool never repairs it implicitly.
    nums = tuple(values[:11]) + tuple(uv1 or ()) + tuple(uv2 or ())
    return struct.pack('<' + 'f' * len(nums), *(0.0 if v == 0 else v for v in nums))


def restore_model(original, corrected_gltf):
    before = geometry.parse_geometry_wmodel(original)
    sources, gltf_sha, buffers_sha = geometry.parse_source_gltf(corrected_gltf)
    geometry.require(len(sources) == len(before['submeshes']), 'Source/WModel primitive count differs')
    has_color = sources[0].has_color0
    geometry.require(all(p.has_color0 == has_color for p in sources), 'Mixed source color presence')
    model = geometry.MODEL_HEADER.unpack_from(original, 16)
    sections = []
    for i in range(model[1]):
        kind, index, offset, size, name = geometry.SECTION_DESC.unpack_from(original, 48 + i * 64)
        sections.append(geometry.Section(kind, index, name, original[16 + offset:16 + offset + size]))
    mesh = next(s.payload for s in sections if s.type_id == 1)
    header = list(geometry.MESH_HEADER.unpack_from(mesh, 16))
    descriptor_start = 16 + geometry.MESH_HEADER.size
    descriptors = [list(geometry.SUBMESH_DESC.unpack_from(mesh, descriptor_start + i * geometry.SUBMESH_DESC.size)) for i in range(header[1])]
    vertex_start = descriptor_start + header[1] * geometry.SUBMESH_DESC.size
    old_stride = header[4]
    index_start = vertex_start + header[5] * old_stride
    metadata_start = len(mesh) - geometry.GEOMETRY_METADATA_SIZE
    unchanged_tail = mesh[index_start:metadata_start]
    blocks, summaries = [], []
    cursor = 0
    for source, target, descriptor in zip(sources, before['submeshes'], descriptors):
        mapping = collections.defaultdict(set)
        source_signs = collections.defaultdict(set)
        for vertex in source.vertices:
            values, color = geometry.transform_source_vertex(vertex, before['sourceToWModelScale'])
            mapping[vertex_key(values, vertex['uv1'], vertex['uv2'])].add(color)
            source_signs[vertex_key(values, vertex['uv1'], vertex['uv2'])].add(values[11])
        transformed = [geometry.transform_source_vertex(v, before['sourceToWModelScale'])[0] for v in source.vertices]
        reflected_indices = tuple(v for i in range(0, len(source.indices), 3) for v in (source.indices[i], source.indices[i + 2], source.indices[i + 1]))
        geometry.require(geometry.triangle_signatures(transformed, reflected_indices) == geometry.triangle_signatures([v['values'] for v in target['vertices']], target['indices']), 'Source/WModel topology differs')
        output = bytearray()
        changed = differing_signs = 0
        for i, vertex in enumerate(target['vertices']):
            matches = mapping.get(vertex_key(vertex['values'], vertex['uv1'], vertex['uv2']), set())
            geometry.require(len(matches) == 1, 'Installed vertex has missing/ambiguous source color')
            color = next(iter(matches))
            differing_signs += vertex['values'][11] not in source_signs[vertex_key(vertex['values'], vertex['uv1'], vertex['uv2'])]
            raw = target['vertexBytes'][i * old_stride:(i + 1) * old_stride]
            tail_offset = 52 if before['hasColor0'] else 48
            output.extend(raw[:48] + (color if has_color else b'') + raw[tail_offset:])
            changed += vertex['color0'] != color
        descriptor[0] = cursor
        blocks.append(bytes(output)); cursor += len(output)
        summaries.append(dict(vertices=len(target['vertices']), changedColorVertices=changed,
                              preservedInstalledHandednessDifferingFromFreshExport=differing_signs))
    header[3] = (header[3] & ~geometry.VF_COLOR0) | (geometry.VF_COLOR0 if has_color else 0)
    header[4] = old_stride + 4 * (int(has_color) - int(before['hasColor0']))
    payload = geometry.MESH_HEADER.pack(*header) + b''.join(geometry.SUBMESH_DESC.pack(*d) for d in descriptors) + b''.join(blocks) + unchanged_tail
    metadata = bytearray(mesh[metadata_start:])
    prefix = list(geometry.GEOMETRY_METADATA_PREFIX.unpack_from(metadata))
    prefix[4] = (prefix[4] & ~geometry.MGEF_COLOR0_PRESERVED_FROM_GLTF) | (geometry.MGEF_COLOR0_PRESERVED_FROM_GLTF if has_color else 0)
    prefix[5] = len(payload)
    metadata[:geometry.GEOMETRY_METADATA_PREFIX.size] = geometry.GEOMETRY_METADATA_PREFIX.pack(*prefix)
    digest_at = geometry.GEOMETRY_METADATA_PREFIX.size
    for i, value in ((0, hashlib.sha256(payload).digest()), (1, gltf_sha), (2, buffers_sha)):
        metadata[digest_at + 32 * i:digest_at + 32 * (i + 1)] = value
    metadata[-32:] = hashlib.sha256(metadata[:-32]).digest()
    content = payload + metadata
    version = geometry.FILE_HEADER.unpack_from(mesh)[2]
    new_mesh = geometry.FILE_HEADER.pack(b'WINT', 1, version, 0, len(content)) + content
    result = geometry.rebuild_wmodel((model[1], model[2], model[3], tuple(model[4:])), sections, new_mesh)
    after = geometry.parse_geometry_wmodel(result)
    geometry.require(after['hasColor0'] == has_color, 'Restored color flag differs')
    for old, new in zip(before['submeshes'], after['submeshes']):
        geometry.require(old['indexBytes'] == new['indexBytes'] and old['bounds'] == new['bounds'], 'Indices or bounds changed')
        for a, b in zip(old['vertices'], new['vertices']):
            geometry.require((a['values'], a['uv1'], a['uv2']) == (b['values'], b['uv1'], b['uv2']), 'Non-color geometry changed')
    material = next(s.payload for s in sections if s.type_id == 2)
    out_material = next(result[16 + off:16 + off + size] for i in range(model[1]) for kind, _, off, size, _ in [geometry.SECTION_DESC.unpack_from(result, 48 + i * 64)] if kind == 2)
    geometry.require(material == out_material, 'Embedded material changed')
    unchanged_colors = before['hasColor0'] == has_color and not any(r['changedColorVertices'] for r in summaries)
    if unchanged_colors:
        result = original
    return result, dict(hasColorBefore=before['hasColor0'], hasColorAfter=has_color, unchangedColors=unchanged_colors,
                        nonColorVertexBytesIdentical=True, topologyIndicesBoundsByteIdentical=True,
                        materialBytesIdentical=True, sourceGeometryJoin=True, submeshes=summaries)


def restore(source_receipt, model, output, stage_directory, report):
    geometry.require(model.resolve() != output.resolve(), 'In-place model changes are forbidden')
    original = model.read_bytes()
    source, proof = stage_source(source_receipt, stage_directory)
    result, summary = restore_model(original, source)
    geometry.require(model.read_bytes() == original, 'Input WModel changed before candidate write')
    summary.update(input=str(model.resolve()), inputSHA256=digest(original), output=str(output.resolve()), outputSHA256=digest(result), nativeSource=proof)
    write_pair(output, result, report, (json.dumps(summary, ensure_ascii=False, indent=2) + '\n').encode('utf-8'))
    return summary


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('source-receipt', 'model', 'output', 'stage-directory', 'report'):
        parser.add_argument('--' + name, type=Path, required=True)
    args = parser.parse_args()
    result = restore(args.source_receipt, args.model, args.output, args.stage_directory, args.report)
    print(json.dumps({k: v for k, v in result.items() if k not in ('nativeSource', 'submeshes')}))
