#!/usr/bin/env python3
"""Recover v868 native foliage matrices and per-instance RNM coordinates.

Ordinary StaticMeshComponent extraction does not include this source layer.
Reuse the existing Bern matrix and component lighting decoders; emit evidence
only, without replacing authoring/runtime files or inferring visibility by name.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import struct
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'BernCastlePipeline'))
import build_bern_castle_foliage_overlay as matrices
import extract_ue3_placements as ue
from extract_source_map_component_lighting import ComponentLightingExtractor, decode_native_lighting


def require(condition, message):
    if not condition:
        raise ValueError(message)


def decode_component(tail, reference, label):
    reader = ue.Reader(tail)
    require(reader.i32() == 1, f'{label}: only one LOD is supported')
    shadow_count = reader.i32()
    require(0 <= shadow_count <= 4096, f'{label}: invalid shadow count')
    reader.read(shadow_count * 4)
    require(reader.i32() == 0, f'{label}: vertex shadows are unsupported')
    require(reader.i32() == 2, f'{label}: RNM texture lightmap required')
    guid_count = reader.i32()
    require(0 <= guid_count <= 4096, f'{label}: invalid light GUID count')
    reader.read(guid_count * 16 + 3 * 16 + 4 * 4)
    require(reader.read(1) == b'\0', f'{label}: vertex color override is unsupported')
    require(reader.read(4) == bytes(4), f'{label}: component footer changed')
    lighting = decode_native_lighting(tail[:reader.offset], reference)
    stride, count = reader.unpack('<II')
    require(stride == 80 and 0 < count <= 1_000_000, f'{label}: invalid instance array')
    require(reader.offset + stride * count == len(tail), f'{label}: native suffix span mismatch')
    instances = [matrices.decode_instance_matrix(reader.unpack('<20f'), f'{label}:{i}')
                 for i in range(count)]
    return lighting, instances


def extract(package_path, logical_name, script_root=None):
    lighting_reader = ComponentLightingExtractor(package_path, logical_name, logical_name)
    package = lighting_reader.package
    require(package.summary.version == 868, 'only the verified v868 native layout is supported')
    visibility = ue.SourceVisibilityResolver(package_path, logical_name, package.summary,
        package.names, package.imports, package.exports, package.logical, script_root)
    components, placements = [], []
    for entry in package.exports:
        if ue.package_ref_name(entry.class_index, package.imports, package.exports) != 'instancedstaticmeshcomponent':
            continue
        require(entry.package_index > 0, 'foliage owner must be an export')
        actor = package.exports[entry.package_index - 1]
        require(ue.package_ref_name(actor.class_index, package.imports, package.exports) == 'instancedfoliageactor',
                'unsupported foliage component owner')
        raw = package.logical[entry.serial_offset:entry.serial_offset + entry.serial_size]
        props, end = ue.parse_tagged_properties(raw, package.names, package.summary.version)
        source_mesh = ue.package_ref_path(ue.property_value(props, 'StaticMesh', 0), package.imports, package.exports)
        require(source_mesh, 'foliage StaticMesh is absent')
        # A nonidentity component transform must be composed explicitly by a
        # future source-layout extension, never silently discarded here.
        for name, identity in [('translation', {'x': 0., 'y': 0., 'z': 0.}),
                               ('scale', 1.), ('scale3d', {'x': 1., 'y': 1., 'z': 1.})]:
            if name in props:
                require(props[name]['value'] == identity, f'{entry.index}: nonidentity {name}')
        require('rotation' not in props, f'{entry.index}: component rotation requires composition')
        source_visibility = visibility.resolve(actor, entry)
        light, instances = decode_component(raw[end:], lighting_reader.reference, str(entry.index))
        components.append(dict(sourceExportIndex0=entry.index, sourceObject=entry.object_name,
            sourceMesh=source_mesh, sourceVisibility=source_visibility, properties=props,
            materialOverrides=ue.material_override_slots(props, package.imports, package.exports),
            serialSHA256=hashlib.sha256(raw).hexdigest(), lighting=light, instanceCount=len(instances)))
        for index, instance in enumerate(instances):
            source_id = f'{logical_name}:foliage:{entry.index}:{index}'
            placements.append(dict(sourcePlacementId=source_id, placementId=matrices.stable_placement_id(source_id) | (1 << 63),
                sourceComponentExportIndex0=entry.index, sourceInstanceIndex=index, sourceMesh=source_mesh,
                visible=source_visibility['visible'], position=instance.position,
                quaternion=instance.quaternion, scale=instance.signed_scale,
                lightmapUVBias=instance.lightmap_uv_bias, shadowmapUVBias=instance.shadowmap_uv_bias))
    require(components and placements, 'no native foliage instances found')
    require(len({r['placementId'] for r in placements}) == len(placements), 'stable ID collision')
    require(hashlib.sha256(package_path.read_bytes()).hexdigest() == package.sha256,
            'source package changed during extraction')
    return dict(schema='lostark.ue3-foliage-placements', formatVersion=1, areaId=logical_name,
        sourcePackage=str(package_path.resolve()), sourcePackageSHA256=package.sha256,
        coordinateSystem='Client meters X,Z,-Y', components=components, placements=placements,
        summary=dict(componentCount=len(components), instanceCount=len(placements),
                     visibleCount=sum(r['visible'] for r in placements)))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--package', required=True)
    parser.add_argument('--package-root', type=Path, required=True)
    parser.add_argument('--umodel', type=Path, required=True)
    parser.add_argument('--script-root', type=Path)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    source = ue.resolve_physical_package(args.umodel, args.package_root, args.package, 'kr')
    result = extract(source, args.package, args.script_root)
    target = args.output.resolve()
    require(not target.is_relative_to(args.package_root.resolve()), 'output must not overwrite original packages')
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(json.dumps(result, ensure_ascii=False, indent=2, allow_nan=False) + '\n', encoding='utf-8')
    print(json.dumps(result['summary']))


if __name__ == '__main__':
    main()
