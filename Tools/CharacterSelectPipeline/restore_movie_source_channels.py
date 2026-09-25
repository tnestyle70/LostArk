"""Restore source-joined movie normals, tangents, handedness and additional UVs.

The sidecar is produced by normalize_source_movie_basis.source_join: every
indexed source corner must match the normalized WModel position and UV0 first.
No further coordinate conversion is applied here.
"""
from dataclasses import replace
import hashlib
import math
from pathlib import Path
import struct
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Tools/ModelAssetConverter"))
import cook_wmodel_geometry_contract as geometry


def restore(data, sidecar):
    parsed = geometry.parse_skinned_uv_wmodel(data)
    mesh = bytearray(parsed["mesh"])
    rows = sidecar["submeshes"]
    assert len(rows) == len(parsed["submeshes"])
    channels, signs = {}, {}
    for index, (row, desc) in enumerate(zip(rows, parsed["submeshes"])):
        assert row["vertexCount"] == desc[1] and row["materialIndex"] == desc[4]
        joined = row["indexedSourceChannels"]
        assert len(joined) == desc[1]
        channels[index] = {name: [v[name] for v in joined]
                           for name in ("TEXCOORD_1", "TEXCOORD_2")
                           if name in row["sourceAttributes"]}
        signs[index] = []
        for vertex, source in enumerate(joined):
            at = parsed["vertexStart"] + desc[0] + vertex * parsed["meshHeader"][4]
            for name, offset in (("NORMAL", 12), ("TANGENT", 32)):
                values = source[name][:3]
                length = math.sqrt(sum(v * v for v in values))
                assert math.isfinite(length) and length > 1e-8
                struct.pack_into("<3f", mesh, at + offset, *(v / length for v in values))
            sign = source["TANGENT"][3]
            assert sign in (-1., 1.)
            signs[index].append(sign)
    sections = [replace(s, payload=bytes(mesh)) if s.type_id == 1 else s
                for s in parsed["sections"]]
    normalized = geometry.rebuild_wmodel(parsed["modelHeader"], sections, bytes(mesh))
    result, receipt = geometry.cook_skinned_basis_uv_contract(normalized, channels, signs)
    receipt.update(sourceModelSha256=hashlib.sha256(data).hexdigest(),
                   sourceGltfSha256=sidecar["sourceGltfSha256"],
                   maxPositionJoinErrorCm=sidecar["maxPositionErrorCm"],
                   maxUv0JoinError=sidecar["maxUv0Error"],
                   outputSha256=hashlib.sha256(result).hexdigest(),
                   sourceBasisRestored=True)
    return result, receipt
