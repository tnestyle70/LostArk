"""Annotate an out-only glTF with proven retail packed parallel tangent bases.

Reads the native position/UV stream headers independently, joins every glTF
vertex by position/UV0, then proves each parallel axis against its packed bytes.
No vertex, index, normal, tangent or buffer byte is repaired or regenerated.
"""
from pathlib import Path
import argparse
import hashlib
import json
import shutil
import struct
import sys
import numpy as np
from scipy.spatial import cKDTree

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT/'Tools/EffectPipeline'), str(ROOT/'Tools/ModelAssetConverter')]
import build_gate3_world_auras as aura
import cook_wmodel_geometry_contract as geometry


def digest(data):
    return hashlib.sha256(data).hexdigest()


def prove(source_receipt, output_gltf, receipt_path):
    for path in (output_gltf, receipt_path):
        if not path.resolve().is_relative_to((ROOT/'out').resolve()):
            raise ValueError('Candidate outputs must remain beneath workspace/out')
    source = json.loads(source_receipt.read_text(encoding='utf-8-sig'))
    gltf = source_receipt.parent/source['gltf']
    doc = json.loads(gltf.read_text(encoding='utf-8-sig'))
    package_path = Path(source['physicalPackagePath'])
    package = aura.source.load_package(package_path, aura.source.ue3.LOSTARK_KR_AES_KEY)
    entry = aura.source.find_export(package, source['fullPath'].split('.', 1)[1])
    raw = package.logical[entry.serial_offset:entry.serial_offset+entry.serial_size]
    cache = {}
    decoded = []
    for mesh in doc['meshes']:
        for primitive in mesh['primitives']:
            arrays = {name: np.array(geometry.accessor_values(doc, gltf.parent, index, cache)[0])
                      for name, index in primitive['attributes'].items()}
            decoded.append((primitive, arrays))
    count = sum(len(arrays['POSITION']) for _, arrays in decoded)
    positions, streams = [], []
    for offset in range(len(raw)-24):
        uv, stride, n, fp, bulkstride, bulkcount = struct.unpack_from('<6I', raw, offset)
        if 1 <= uv <= 4 and fp in (0, 1) and stride == 8+uv*(8 if fp else 4) and stride == bulkstride and n == bulkcount == count and offset+24+n*stride <= len(raw):
            streams.append((offset, uv, stride, fp))
        if (uv, stride, n, fp) == (12, count, 12, count) and offset+16+count*12 <= len(raw):
            positions.append(offset)
    if len(positions) != 1 or len(streams) != 1:
        raise ValueError(f'Native stream headers are ambiguous: {positions}, {streams}')
    position_offset = positions[0]+16
    uv_offset, uv_count, stride, fp = streams[0]
    vertex_offset = uv_offset+24
    source_positions = np.frombuffer(raw, dtype='<f4', count=count*3, offset=position_offset).reshape(-1, 3)[:, [0, 2, 1]]*.01
    source_uv = np.array([struct.unpack_from('<2f' if fp else '<2e', raw, vertex_offset+i*stride+8) for i in range(count)])
    native_join = np.concatenate([source_positions, source_uv], axis=1)
    tree = cKDTree(native_join)
    proofs = []
    max_join = 0.
    for primitive_index, (primitive, arrays) in enumerate(decoded):
        joined = np.concatenate([arrays['POSITION'], arrays['TEXCOORD_0']], axis=1)
        distances, _ = tree.query(joined)
        max_join = max(max_join, float(max(distances)))
        if max(distances) > 1e-4:
            raise ValueError('Native position/UV0 stream does not match the complete source glTF')
        bad = np.flatnonzero(np.all(np.cross(arrays['NORMAL'], arrays['TANGENT'][:, :3]) == 0, axis=1))
        for index in bad:
            matches = []
            for native_index in tree.query_ball_point(joined[index], 1e-4):
                at = vertex_offset+native_index*stride
                packed_t = list(raw[at:at+4]); packed_n = list(raw[at+4:at+8])
                t = (np.array(packed_t[:3])/127.5-1)[[0, 2, 1]]
                n = (np.array(packed_n[:3])/127.5-1)[[0, 2, 1]]
                if not np.all(np.cross(t, n) == 0) or min(np.linalg.norm(t), np.linalg.norm(n)) == 0:
                    continue
                t /= np.linalg.norm(t); n /= np.linalg.norm(n)
                if max(np.max(np.abs(t-arrays['TANGENT'][index, :3])), np.max(np.abs(n-arrays['NORMAL'][index]))) > 1e-6:
                    continue
                matches.append(dict(nativeVertex=native_index, serialOffset=at, tangentPacked=packed_t, normalPacked=packed_n))
            if not matches:
                raise ValueError(f'No native packed proof for parallel primitive {primitive_index} vertex {index}')
            proofs.append(dict(primitive=primitive_index, vertex=int(index), nativeMatches=matches,
                               normal=arrays['NORMAL'][index].tolist(), tangent=arrays['TANGENT'][index].tolist()))
        if len(bad):
            primitive.setdefault('extras', {})['lostarkNativeParallelBasis'] = dict(vertexIndices=bad.tolist(), nativeSerialSHA256=digest(raw))
    if not proofs:
        raise ValueError('No native parallel vertices require annotation')
    output_gltf.parent.mkdir(parents=True, exist_ok=True)
    for buffer in doc['buffers']:
        uri = Path(buffer['uri'])
        if uri.is_absolute() or '..' in uri.parts:
            raise ValueError('Source buffer URI must remain relative')
        destination = output_gltf.parent/uri
        destination.parent.mkdir(parents=True, exist_ok=True)
        if (gltf.parent/uri).resolve() != destination.resolve():
            shutil.copy2(gltf.parent/uri, destination)
    output_gltf.write_text(json.dumps(doc, indent=2)+'\n', encoding='utf-8')
    geometry.parse_source_gltf(output_gltf)
    receipt = dict(schema='lostark.retail-native-parallel-basis-proof', sourceObject=source['fullPath'],
        sourceGltf=str(gltf), sourceGltfSha256=digest(gltf.read_bytes()), sourcePackage=str(package_path),
        sourcePackageSha256=digest(package_path.read_bytes()), nativeSerialSHA256=digest(raw),
        nativeExportSerialOffset=entry.serial_offset, nativeExportSerialBytes=len(raw),
        nativePositionHeaderOffset=positions[0], nativeUvHeaderOffset=uv_offset, nativeVertexCount=count,
        nativeUvCount=uv_count, nativeUvStride=stride, nativeFloatUv=bool(fp),
        positionBasis='UE cm -> UModel glTF meters [x,z,y]', completePositionUv0JoinMaxError=max_join,
        proofs=proofs, outputGltf=str(output_gltf), outputGltfSha256=digest(output_gltf.read_bytes()),
        geometryBuffersByteIdentical=True, nativeParallelBasisPreserved=True,
        behavior='Preserve original parallel N/T and original glTF tangent.w; zero binormal remains zero. No geometric tangent reconstruction.',
        formatReference='https://github.com/gildor2/UEViewer/blob/master/Unreal/UnrealMesh/UnMesh3.cpp: FStaticMeshVertexStream3 / FStaticMeshUVStream3 / FStaticMeshUVItem3')
    receipt_path.parent.mkdir(parents=True, exist_ok=True)
    receipt_path.write_text(json.dumps(receipt, indent=2)+'\n', encoding='utf-8')
    return receipt


def verify_and_stage(source_receipt, staged_gltf, stage_root):
    """Re-prove a receipt-pinned annotation before a common map cook consumes it."""
    source = json.loads(source_receipt.read_text(encoding='utf-8-sig'))
    declaration = source.get('nativeParallelBasisProof')
    if not isinstance(declaration, dict) or set(declaration) != {'path', 'sha256'}:
        raise ValueError('Native parallel proof declaration must pin path and SHA-256')
    relative = Path(declaration['path'])
    if relative.is_absolute() or '..' in relative.parts:
        raise ValueError('Native parallel proof path must stay beneath its source export')
    proof_path = source_receipt.parent/relative
    if digest(proof_path.read_bytes()).casefold() != declaration['sha256'].casefold():
        raise ValueError('Native parallel proof receipt changed')
    expected = json.loads(proof_path.read_text(encoding='utf-8-sig'))
    verified_gltf = stage_root/'original'/source['gltf']
    actual = prove(source_receipt, verified_gltf, stage_root/'verified.receipt.json')
    # Paths may change on another PC; all native hashes, stream offsets, joins
    # and individual packed bytes must remain the same.
    fields = ('schema', 'sourceObject', 'sourceGltfSha256', 'sourcePackageSha256',
              'nativeSerialSHA256', 'nativeExportSerialOffset', 'nativeExportSerialBytes',
              'nativePositionHeaderOffset', 'nativeUvHeaderOffset', 'nativeVertexCount',
              'nativeUvCount', 'nativeUvStride', 'nativeFloatUv',
              'completePositionUv0JoinMaxError', 'proofs', 'geometryBuffersByteIdentical',
              'nativeParallelBasisPreserved')
    if any(expected.get(field) != actual[field] for field in fields):
        raise ValueError('Native parallel proof no longer matches the retail source')
    original = json.loads(verified_gltf.read_text(encoding='utf-8'))
    staged = json.loads(staged_gltf.read_text(encoding='utf-8-sig'))
    original_primitives = [p for m in original['meshes'] for p in m['primitives']]
    staged_primitives = [p for m in staged['meshes'] for p in m['primitives']]
    if len(original_primitives) != len(staged_primitives):
        raise ValueError('Staged source primitive count changed')
    for original_primitive, primitive in zip(original_primitives, staged_primitives):
        annotation = original_primitive.get('extras', {}).get('lostarkNativeParallelBasis')
        if annotation:
            primitive.setdefault('extras', {})['lostarkNativeParallelBasis'] = annotation
    result = stage_root/'staged'/staged_gltf.name
    result.parent.mkdir(parents=True, exist_ok=True)
    for buffer in staged['buffers']:
        uri = Path(buffer['uri'])
        if uri.is_absolute() or '..' in uri.parts:
            raise ValueError('Staged buffer URI must remain relative')
        destination = result.parent/uri
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(staged_gltf.parent/uri, destination)
    result.write_text(json.dumps(staged, indent=2)+'\n', encoding='utf-8')
    return verified_gltf, result


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--source-receipt', required=True, type=Path)
    p.add_argument('--output-gltf', required=True, type=Path)
    p.add_argument('--receipt', required=True, type=Path)
    a = p.parse_args()
    result = prove(a.source_receipt.resolve(), a.output_gltf.resolve(), a.receipt.resolve())
    print(json.dumps(dict(nativeVertexCount=result['nativeVertexCount'], parallelVertices=len(result['proofs']), receipt=str(a.receipt))))
