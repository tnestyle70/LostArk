"""Bake source placement reflections into new, positive-TRS static WModels.

The source model directory is retained so embedded material paths keep their
meaning. Existing outputs are reused only when byte-identical, never replaced.
The WorldSequence generator owns object-resource cloning and binding changes.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
from pathlib import Path, PurePosixPath
import struct
import sys
import tempfile
from typing import Sequence

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Tools/ModelAssetConverter"))
import cook_wmodel_geometry_contract as geometry
from patch_wmodel_dye import parse_material_section, wstr


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def reflection_tag(signs: Sequence[int]) -> str:
    require(len(signs) == 3 and all(type(s) is int and s in (-1, 1) for s in signs),
            "Reflection needs three integer axis signs")
    require(any(s < 0 for s in signs), "A reflected variant needs a negative axis")
    return "".join("n" if s < 0 else "p" for s in signs)


def asset_path(root: Path, asset_id: str) -> Path:
    relative = PurePosixPath(asset_id)
    require(bool(asset_id) and "\\" not in asset_id and ":" not in asset_id
            and not relative.is_absolute() and ".." not in relative.parts,
            "Expected a Resources-relative asset ID: " + asset_id)
    target = (root / relative).resolve()
    require(target.is_relative_to(root.resolve()), "Asset escaped its root")
    return target


def reflected_asset_id(model_asset_id: str, signs: Sequence[int]) -> str:
    tag = reflection_tag(signs)
    relative = PurePosixPath(model_asset_id)
    require(relative.suffix == ".wmodel", "Reflection input must be a WModel")
    return str(relative.with_name(relative.stem + "__reflect_" + tag + ".wmodel"))


def section_range(data: bytes, kind: int) -> tuple[int, int]:
    count = geometry.MODEL_HEADER.unpack_from(data, geometry.FILE_HEADER.size)[1]
    matches = []
    for row in range(count):
        fields = geometry.SECTION_DESC.unpack_from(
            data, geometry.FILE_HEADER.size + geometry.MODEL_HEADER.size
            + row * geometry.SECTION_DESC.size)
        if fields[0] == kind:
            matches.append((geometry.FILE_HEADER.size + fields[2], fields[3]))
    require(len(matches) == 1, "Expected one section of kind " + str(kind))
    return matches[0]


def bake_bytes(source: bytes, signs: Sequence[int]) -> bytes:
    """Use the existing strict geometry contract before and after byte edits."""
    reflection_tag(signs)
    geometry.parse_geometry_wmodel(source)
    mesh_begin, mesh_size = section_range(source, 1)
    mesh = bytearray(source[mesh_begin:mesh_begin + mesh_size])
    header = geometry.MESH_HEADER.unpack_from(mesh, geometry.FILE_HEADER.size)
    count, stride, vertex_count, index_count, index_stride = (
        header[1], header[4], header[5], header[6], header[7])
    vertex_begin = (geometry.FILE_HEADER.size + geometry.MESH_HEADER.size
                    + count * geometry.SUBMESH_DESC.size)
    index_begin = vertex_begin + vertex_count * stride
    bounds_begin = index_begin + index_count * index_stride
    metadata_begin = bounds_begin + count * geometry.BOUNDS_V1.size
    parity = math.prod(signs)
    for row in range(vertex_count):
        begin = vertex_begin + row * stride
        for offset in (0, 12, 32):  # Position, normal, tangent XYZ; UV bytes stay put.
            values = struct.unpack_from("<3f", mesh, begin + offset)
            struct.pack_into("<3f", mesh, begin + offset,
                             *(v * s for v, s in zip(values, signs)))
        tangent_w = struct.unpack_from("<f", mesh, begin + 44)[0]
        struct.pack_into("<f", mesh, begin + 44, tangent_w * parity)
    if parity < 0:
        for triangle in range(index_count // 3):
            second = index_begin + (triangle * 3 + 1) * index_stride
            third = second + index_stride
            mesh[second:second + index_stride], mesh[third:third + index_stride] = (
                mesh[third:third + index_stride], mesh[second:second + index_stride])
    for row in range(count):
        begin = bounds_begin + row * geometry.BOUNDS_V1.size
        bounds = list(geometry.BOUNDS_V1.unpack_from(mesh, begin))
        for axis, sign in enumerate(signs):
            if sign < 0:
                bounds[axis], bounds[axis + 3] = -bounds[axis + 3], -bounds[axis]
                bounds[axis + 6] = -bounds[axis + 6]
        geometry.BOUNDS_V1.pack_into(mesh, begin, *bounds)
    digest_begin = metadata_begin + geometry.GEOMETRY_METADATA_PREFIX.size
    mesh[digest_begin:digest_begin + 32] = hashlib.sha256(
        mesh[geometry.FILE_HEADER.size:metadata_begin]).digest()
    # Source digests retain their original evidence role. This transform is the
    # geometry tool now; it does not claim authenticated UPK source fidelity.
    mesh[digest_begin + 6 * 32:digest_begin + 7 * 32] = hashlib.sha256(
        Path(__file__).read_bytes()).digest()
    mesh[-32:] = hashlib.sha256(mesh[metadata_begin:-32]).digest()
    result = source[:mesh_begin] + bytes(mesh) + source[mesh_begin + mesh_size:]
    geometry.parse_geometry_wmodel(result)
    return result


def verify_bytes(source: bytes, candidate: bytes, signs: Sequence[int]) -> dict:
    original = geometry.parse_geometry_wmodel(source)
    reflected = geometry.parse_geometry_wmodel(candidate)
    require(len(source) == len(candidate), "Reflection changed the container size")
    begin, size = section_range(source, 1)
    require(source[:begin] == candidate[:begin]
            and source[begin + size:] == candidate[begin + size:],
            "Reflection modified non-mesh data")
    for key in ("vertexFlags", "vertexStride", "indexStride", "evidenceFlags",
                "sourceToWModelScale", "geometryPreScale", "sourceGltfSha256",
                "sourceBufferSetSha256"):
        require(original[key] == reflected[key], "Geometry contract changed: " + key)
    parity = math.prod(signs)
    vertices = indices = 0
    for left, right in zip(original["submeshes"], reflected["submeshes"]):
        for key in ("name", "materialIndex", "materialHash"):
            require(left[key] == right[key], "Submesh identity changed: " + key)
        require(len(left["vertices"]) == len(right["vertices"]), "Vertex count changed")
        stride = original["vertexStride"]
        for row, (lv, rv) in enumerate(zip(left["vertices"], right["vertices"])):
            a, b = lv["values"], rv["values"]
            for offset in (0, 3, 8):
                require(all(b[offset + axis] == a[offset + axis] * signs[axis]
                            for axis in range(3)), "A reflected vertex channel differs")
            require(b[11] == a[11] * parity, "Reflected tangent W differs")
            base = row * stride
            require(left["vertexBytes"][base + 24:base + 32]
                    == right["vertexBytes"][base + 24:base + 32]
                    and left["vertexBytes"][base + 48:base + stride]
                    == right["vertexBytes"][base + 48:base + stride],
                    "UV or color channel bytes changed")
        expected = list(left["indices"])
        if parity < 0:
            for i in range(0, len(expected), 3):
                expected[i + 1], expected[i + 2] = expected[i + 2], expected[i + 1]
        require(tuple(expected) == right["indices"], "Triangle winding differs")
        vertices += len(left["vertices"])
        indices += len(expected)
    material_begin, material_size = section_range(source, 2)
    return dict(vertexCount=vertices, indexCount=indices,
                submeshCount=len(original["submeshes"]), determinant=parity,
                materialSha256=hashlib.sha256(
                    source[material_begin:material_begin + material_size]).hexdigest(),
                geometryPreScale=reflected["geometryPreScale"],
                payloadSha256=reflected["payloadSha256"].hex(),
                metadataIdentitySha256=reflected["metadataIdentitySha256"].hex(),
                geometryChannelMaxError=0.0, uvAndColorBytesPreserved=True,
                materialSectionBytesPreserved=True, windingParityVerified=True)


def material_paths(resources_root: Path, model_asset_id: str, source: bytes) -> list[str]:
    begin, size = section_range(source, 2)
    _, _, entries = parse_material_section(source[begin:begin + size])
    paths = set()
    for entry in entries:
        for raw in entry[3:12]:
            stored = wstr(raw).replace("\\", "/")
            if not stored:
                continue
            require(not PurePosixPath(stored).is_absolute() and ":" not in stored
                    and ".." not in PurePosixPath(stored).parts,
                    "Expected a local material path: " + stored)
            relative = str(PurePosixPath(model_asset_id).parent / stored)
            path = asset_path(resources_root, relative)
            if not path.is_file():
                path = asset_path(resources_root, stored)
                relative = stored
            if not path.is_file():
                # Match WMaterialReader::ResolveBelowAssetRoot for older input
                # strings; preserve the embedded bytes, including their prefix.
                for prefix in ("Resource/LostArk/", "Resource/", "Resources/"):
                    if prefix not in stored:
                        continue
                    relocated = stored.split(prefix, 1)[1]
                    relocated_path = asset_path(resources_root, relocated)
                    if relocated_path.is_file():
                        path, relative = relocated_path, relocated
                        break
            require(path.is_file(), "Missing embedded material input: " + relative)
            paths.add(relative)
    return sorted(paths)


def write_new_or_equal(path: Path, payload: bytes) -> bool:
    """Publish a complete new file without ever replacing an existing path."""
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.exists():
        require(path.is_file() and path.read_bytes() == payload,
                "Preserve existing, different output: " + str(path))
        return False
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(dir=path.parent, prefix=".reflect-", delete=False) as stream:
            temporary = Path(stream.name)
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        try:
            os.link(temporary, path)
        except FileExistsError:
            require(path.is_file() and path.read_bytes() == payload,
                    "Concurrent different output: " + str(path))
            return False
        return True
    finally:
        if temporary is not None:
            temporary.unlink(missing_ok=True)


def bake_reflected_asset(resources_root: Path, output_root: Path,
                         model_asset_id: str, reflection_signs: Sequence[int]) -> dict:
    source_path = asset_path(resources_root, model_asset_id)
    candidate_id = reflected_asset_id(model_asset_id, reflection_signs)
    require(candidate_id != model_asset_id, "Source overwrite is forbidden")
    source = source_path.read_bytes()
    candidate = bake_bytes(source, reflection_signs)
    verification = verify_bytes(source, candidate, reflection_signs)
    paths = material_paths(resources_root, model_asset_id, source)
    write_new_or_equal(asset_path(output_root, candidate_id), candidate)
    return dict(sourceModelAssetId=model_asset_id, modelAssetId=candidate_id,
                reflectionSigns=list(reflection_signs), tag=reflection_tag(reflection_signs),
                sourceSha256=hashlib.sha256(source).hexdigest(),
                candidateSha256=hashlib.sha256(candidate).hexdigest(),
                materialResourcePaths=paths, verification=verification)


def prepare_requests(requests: list[dict], resources_root: Path, output_root: Path) -> dict:
    variants = {}
    occurrences = []
    for request in requests:
        signs = request["reflectionSigns"]
        tag = reflection_tag(signs)
        scale = request["scale"]
        require(len(scale) == 3 and all(math.isfinite(v) and v != 0 for v in scale)
                and [1 if v > 0 else -1 for v in scale] == list(signs),
                "Request reflection does not match its source scale")
        key = (request["modelAssetId"], tag)
        if key not in variants:
            variants[key] = bake_reflected_asset(
                resources_root, output_root, request["modelAssetId"], signs)
        variant = variants[key]
        original = geometry.parse_geometry_wmodel(
            asset_path(resources_root, request["modelAssetId"]).read_bytes())
        candidate = geometry.parse_geometry_wmodel(
            asset_path(output_root, variant["modelAssetId"]).read_bytes())
        max_error = 0.0
        for left, right in zip(original["submeshes"], candidate["submeshes"]):
            for lv, rv in zip(left["vertices"], right["vertices"]):
                for axis in range(3):
                    max_error = max(max_error, abs(lv["values"][axis] * scale[axis]
                                                  - rv["values"][axis] * abs(scale[axis])))
        require(max_error == 0.0, "Positive scale and baked geometry differ")
        occurrences.append(dict(sequenceId=request["sequenceId"],
                                instanceId=request["instanceId"], slotId=request["slotId"],
                                objectId=request["objectId"],
                                reflectedObjectId=request["objectId"] + ".reflect." + tag,
                                modelAssetId=variant["modelAssetId"],
                                positiveScale=[abs(v) for v in scale],
                                signedScaleGeometryMaxError=max_error))
    return dict(formatVersion=1, variants=list(variants.values()), occurrences=occurrences)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--requests", required=True, type=Path)
    parser.add_argument("--resources-root", type=Path, default=ROOT / "Client/Bin/Resources")
    parser.add_argument("--output-root", required=True, type=Path)
    parser.add_argument("--mapping", required=True, type=Path)
    args = parser.parse_args()
    requests = json.loads(args.requests.read_text(encoding="utf-8"))
    result = prepare_requests(requests, args.resources_root, args.output_root)
    write_new_or_equal(args.mapping, (json.dumps(result, ensure_ascii=False, indent=2) + "\n").encode("utf-8"))
    print(json.dumps(dict(variants=len(result["variants"]), occurrences=len(result["occurrences"]),
                          mapping=str(args.mapping)), ensure_ascii=False))


if __name__ == "__main__":
    main()
