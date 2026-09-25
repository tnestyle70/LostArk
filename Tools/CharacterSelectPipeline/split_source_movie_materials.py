"""Stage one source material slot per skinned movie resource without rebaking poses."""
from pathlib import Path
from dataclasses import replace
import hashlib
import struct
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'Tools/ModelAssetConverter'))
import cook_wmodel_geometry_contract as geometry
import verify_dimensionmaster_summon_bind_pose as wm


def split_material(data: bytes, material_slot: int) -> tuple[bytes, dict]:
    parsed = geometry.parse_skinned_uv_wmodel(data)
    mesh = parsed['mesh']; header = list(parsed['meshHeader'])
    if parsed['versionMinor'] not in (0, 3, 5, 6):
        raise ValueError('Unsupported skinned movie stream version')
    kept = [(i, list(row)) for i, row in enumerate(parsed['submeshes']) if row[4] == material_slot]
    if not kept:
        raise ValueError(f'Source material slot {material_slot} has no geometry')
    vertex_start = parsed['vertexStart']; index_start = parsed['indexStart']
    bone_start = index_start + header[6] * header[7]
    bone_end = bone_start + header[2] * wm.MESH_BONE.size
    vertices = bytearray(); indices = bytearray(); bounds = bytearray(); rows = []
    for original_index, row in kept:
        vertex = mesh[vertex_start+row[0]:vertex_start+row[0]+row[1]*header[4]]
        index = mesh[index_start+row[2]:index_start+row[2]+row[3]*header[7]]
        row[0], row[2] = len(vertices), len(indices)
        vertices += vertex; indices += index; rows.append(row)
        if header[8]:
            at = bone_end + original_index * geometry.BOUNDS_V1.size
            bounds += mesh[at:at+geometry.BOUNDS_V1.size]
    header[1], header[5], header[6] = len(rows), len(vertices)//header[4], len(indices)//header[7]
    content = wm.MESH_HEADER.pack(*header) + b''.join(wm.SUBMESH_DESC.pack(*r) for r in rows)
    content += vertices + indices + mesh[bone_start:bone_end] + bounds
    nested = list(wm.FILE_HEADER.unpack_from(mesh)); nested[-1] = len(content)
    if parsed['versionMinor'] in (3, 6):
        blocks, aggregate = [], 0
        for index, row in kept:
            channels = parsed['uvRows'][index]
            mask = (geometry.VF_TEXCOORD1 if 'TEXCOORD_1' in channels else 0) | (geometry.VF_TEXCOORD2 if 'TEXCOORD_2' in channels else 0)
            aggregate |= mask
            blocks.append(struct.pack('<II', row[1], mask))
            for name in ('TEXCOORD_1', 'TEXCOORD_2'):
                if name in channels:
                    blocks.append(b''.join(struct.pack('<2f', *uv) for uv in channels[name]))
        header[3] = (header[3] & ~(geometry.VF_TEXCOORD1 | geometry.VF_TEXCOORD2)) | aggregate
        content = wm.MESH_HEADER.pack(*header) + content[wm.MESH_HEADER.size:]
        if aggregate:
            payload = b''.join(blocks)
            content += geometry.SKINNED_UV_HEADER.pack(b'WUVS', 1, len(kept), len(payload), hashlib.sha256(payload).digest()) + payload
        else:
            nested[2] = 5 if parsed['versionMinor'] == 6 else 0
        nested[-1] = len(content)
    result_mesh = wm.FILE_HEADER.pack(*nested) + content
    sections = [replace(s, payload=result_mesh) if s.type_id == 1 else s for s in parsed['sections']]
    result = geometry.rebuild_wmodel(parsed['modelHeader'], sections, result_mesh)
    readback = geometry.parse_skinned_uv_wmodel(result)
    assert readback['mesh'][readback['vertexStart']:readback['indexStart']] == vertices
    for original, after in zip(parsed['sections'], readback['sections']):
        if original.type_id != 1:
            assert original.payload == after.payload, 'Material/skeleton/animation changed during geometry subset'
    return result, dict(sourceMaterialSlot=material_slot, originalSubmeshIndices=[i for i,_ in kept],
        vertexCount=header[5], indexCount=header[6], skeletonMaterialAnimationsByteIdentical=True,
        sourceSha256=hashlib.sha256(data).hexdigest(), outputSha256=hashlib.sha256(result).hexdigest())
