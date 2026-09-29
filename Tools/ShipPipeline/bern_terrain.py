"""Static WModel terrain helpers shared by the Bern sea navigation and island installers.

Read-only: parses the installed static WModel (WMOD/WMSH layout of the runtime
CModel decoder) and rasterises triangle heights onto a plane grid. Units are
metres after the loader's 0.01 model scale.
"""
import os
import struct

import numpy as np

MODEL_SCALE = 0.01


def read_static_mesh(path):
    """Returns (vertices Nx3 float32 in model units, triangles Mx3 int32)."""
    with open(path, 'rb') as handle:
        payload = handle.read()
    magic, major, _minor, flags, content_size = struct.unpack_from('<4sHHII', payload, 0)
    if magic != b'WINT' or major != 1 or content_size != len(payload) - 16:
        raise ValueError('WModel outer header is invalid: %s' % path)
    content = 16
    model = struct.unpack_from('<4sIIIIIII', payload, content)
    if model[0] != b'WMOD':
        raise ValueError('WMOD header is invalid: %s' % path)
    mesh = None
    for index in range(model[1]):
        section_type, _i, offset, size, _name = struct.unpack_from('<IIQQ40s', payload, content + 32 + index * 64)
        if section_type == 1:
            mesh = payload[content + offset: content + offset + size]
    if mesh is None:
        raise ValueError('WModel has no mesh section: %s' % path)
    header = struct.unpack_from('<4sIIIIIIIB3s', mesh, 16)
    submeshes, bones, _flags, stride, vertex_count, index_count, index_stride, _has_bounds = header[1:9]
    if bones != 0:
        raise ValueError('terrain helper expects a static model: %s' % path)
    vertex_offset = 52 + submeshes * 48
    vertices = np.empty((vertex_count, 3), dtype=np.float32)
    raw = np.frombuffer(mesh, dtype=np.uint8, count=vertex_count * stride, offset=vertex_offset).reshape(vertex_count, stride)
    vertices[:] = raw[:, :12].copy().view(np.float32).reshape(vertex_count, 3)
    index_offset = vertex_offset + vertex_count * stride
    dtype = np.uint16 if index_stride == 2 else np.uint32
    indices = np.frombuffer(mesh, dtype=dtype, count=index_count, offset=index_offset).astype(np.int32)
    return vertices, indices.reshape(-1, 3)


def world_triangles(path, position, scale=(1.0, 1.0, 1.0)):
    """Triangles (M,3,3) in world metres for an identity-rotation placement."""
    vertices, triangles = read_static_mesh(path)
    world = np.empty_like(vertices, dtype=np.float64)
    world[:, 0] = position[0] + vertices[:, 0] * MODEL_SCALE * scale[0]
    world[:, 1] = position[1] + vertices[:, 1] * MODEL_SCALE * scale[1]
    world[:, 2] = position[2] + vertices[:, 2] * MODEL_SCALE * scale[2]
    return world[triangles]


def rasterise_max_height(grid, triangles, x0, z0, cell, width, height):
    """Writes the highest triangle surface over each cell centre into grid (float32, -inf empty)."""
    for tri in triangles:
        min_x = min(tri[0][0], tri[1][0], tri[2][0])
        max_x = max(tri[0][0], tri[1][0], tri[2][0])
        min_z = min(tri[0][2], tri[1][2], tri[2][2])
        max_z = max(tri[0][2], tri[1][2], tri[2][2])
        gx0 = max(int(np.floor((min_x - x0) / cell)), 0)
        gx1 = min(int(np.floor((max_x - x0) / cell)), width - 1)
        gz0 = max(int(np.floor((min_z - z0) / cell)), 0)
        gz1 = min(int(np.floor((max_z - z0) / cell)), height - 1)
        if gx0 > gx1 or gz0 > gz1:
            continue
        ax, ay, az = tri[0]
        bx, by, bz = tri[1]
        cx, cy, cz = tri[2]
        denominator = (bz - cz) * (ax - cx) + (cx - bx) * (az - cz)
        if abs(denominator) < 1e-12:
            continue
        xs = x0 + (np.arange(gx0, gx1 + 1) + 0.5) * cell
        zs = z0 + (np.arange(gz0, gz1 + 1) + 0.5) * cell
        px, pz = np.meshgrid(xs, zs)
        w0 = ((bz - cz) * (px - cx) + (cx - bx) * (pz - cz)) / denominator
        w1 = ((cz - az) * (px - cx) + (ax - cx) * (pz - cz)) / denominator
        w2 = 1.0 - w0 - w1
        inside = (w0 >= -1e-6) & (w1 >= -1e-6) & (w2 >= -1e-6)
        if not inside.any():
            continue
        py = w0 * ay + w1 * by + w2 * cy
        view = grid[gz0:gz1 + 1, gx0:gx1 + 1]
        np.maximum(view, np.where(inside, py, -np.inf).astype(np.float32), out=view)
    return grid


def new_height_grid(width, height):
    return np.full((height, width), -np.inf, dtype=np.float32)
