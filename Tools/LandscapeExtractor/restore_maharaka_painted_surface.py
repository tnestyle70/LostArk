"""Recook only admitted Maharaka terrain with its verified painted layer UVs.

Does not change source heights, holes, placements, water, colours, or profiles.
--restore-coordinate-basis repairs old locally mirrored cooks against source.
Requires the prior source-layer bake receipt; never supplies guessed colours.
Stage first; --apply preserves originals and rolls back this installation on error.
"""
import argparse
from collections import Counter
import hashlib
import json
import os
from pathlib import Path
import re
import struct
import subprocess

import extract_ue3_landscape as landscape

ROOT = Path(__file__).resolve().parents[2]
RESOURCE = ROOT / 'Client/Bin/Resources'
AREA = 'LV_OCN_EVENTIS_MHP'


def digest(data):
    return hashlib.sha256(data).hexdigest()


def triangles(data):
    """Compare oriented positions independent of submesh/vertex duplication."""
    assert data[:4] == b'WINT' and data[16:20] == b'WMOD'
    count = struct.unpack_from('<I', data, 20)[0]
    sections = [struct.unpack_from('<IIQQ40s', data, 48+i*64) for i in range(count)]
    section, = [s for s in sections if s[0] == 1]
    offset = 16 + section[2] + 16
    h = struct.unpack_from('<4sIIIIIIIB3s', data, offset)
    assert h[0] == b'WMSH' and h[2] == 0 and h[7] in (2, 4)
    desc = [struct.unpack_from('<IIIIIQ20s', data, offset+36+i*48) for i in range(h[1])]
    start = offset+36+h[1]*48
    index_start = start+h[4]*h[5]
    assert index_start+h[6]*h[7] <= 16+section[2]+section[3]
    result = Counter()
    for vo, vc, io, ic, _, _, _ in desc:
        assert ic % 3 == 0 and vo+vc*h[4] <= h[5]*h[4]
        points = [tuple(round(x, 3) for x in struct.unpack_from('<3f', data, start+vo+i*h[4])) for i in range(vc)]
        indices = struct.unpack_from('<'+('H' if h[7] == 2 else 'I')*ic, data, index_start+io)
        assert all(i < vc for i in indices)
        for i in range(0, ic, 3):
            tri = tuple(points[k] for k in indices[i:i+3])
            # Rotation is allowed; reversal is not (culling must stay unchanged).
            result[min(tri, tri[1:]+tri[:1], tri[2:]+tri[:2])] += 1
    return result


def validate_source_geometry(data, component, proxy):
    """Validate the cooked consumer, not just glTF or the previous bad model."""
    assert data[:4] == b'WINT' and data[16:20] == b'WMOD'
    sections = [struct.unpack_from('<IIQQ40s', data, 48+i*64)
                for i in range(struct.unpack_from('<I', data, 20)[0])]
    section, = [s for s in sections if s[0] == 1]
    offset = 32 + section[2]
    h = struct.unpack_from('<4sIIIIIIIB3s', data, offset)
    assert h[0] == b'WMSH' and h[1] == 1 and h[2] == 0 and h[4] == 48
    assert h[7] in (2, 4)
    start = offset + 36 + 48
    expected = landscape.component_positions(component, proxy)
    grid = component.component_size_quads+1
    normals, tangents = landscape.component_normals_and_tangents(component, expected, grid)
    assert h[5] == len(expected)
    logical_ids = []
    max_position_error = max_normal_error = 0.0
    handedness = set()
    for i in range(h[5]):
        v = struct.unpack_from('<12f', data, start+i*48)
        x, y = (round(v[k]*(grid-1)) for k in (6, 7))
        assert 0 <= x < grid and 0 <= y < grid
        assert abs(v[6]-x/(grid-1)) < 1e-6 and abs(v[7]-y/(grid-1)) < 1e-6
        logical = y*grid+x
        logical_ids.append(logical)
        max_position_error = max(max_position_error,
            max(abs(v[k]*.01-expected[logical][k]) for k in range(3)))
        max_normal_error = max(max_normal_error,
            max(abs(v[k+3]-normals[logical][k]) for k in range(3)))
        assert max(abs(v[k+8]-tangents[logical][k]) for k in range(3)) < 1e-5
        handedness.add(v[11])
    assert len(set(logical_ids)) == len(expected)
    assert max_position_error < 1e-5 and max_normal_error < 1e-5
    assert handedness == {1.0}, 'cooked tangent frame changed handedness'
    indices = struct.unpack_from('<'+('H' if h[7] == 2 else 'I')*h[6], data, start+h[5]*48)
    canonical = lambda tri: min(tri, tri[1:]+tri[:1], tri[2:]+tri[:2])
    actual = Counter(canonical(tuple(logical_ids[k] for k in indices[i:i+3]))
                     for i in range(0, len(indices), 3))
    desired = Counter()
    for y in range(grid-1):
        for x in range(grid-1):
            if landscape.quad_is_hole(component, x, y):
                continue
            a = y*grid+x
            for tri in ((a, a+1, a+grid), (a+1, a+grid+1, a+grid)):
                desired[canonical(tri)] += 1
    assert actual == desired, 'cooked winding/topology differs from source'
    return dict(vertexCount=h[5], triangleCount=h[6]//3,
                maximumPositionErrorMeters=max_position_error,
                maximumNormalError=max_normal_error, tangentHandedness=sorted(handedness))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--package', type=Path, required=True)
    parser.add_argument('--bake-report', type=Path, required=True)
    parser.add_argument('--install-receipt', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--apply', action='store_true')
    parser.add_argument('--restore-coordinate-basis', action='store_true',
                        help='Explicitly replace old Z-mirrored cooks with source-validated geometry.')
    args = parser.parse_args()
    output = args.output.resolve()
    assert output.is_relative_to((ROOT/'out').resolve())
    report = json.loads(args.bake_report.read_text(encoding='utf8'))
    receipt = json.loads(args.install_receipt.read_text(encoding='utf8'))
    hashes = {str(Path(x['installed']).resolve()).casefold(): x['sha256'] for x in receipt['files']}
    catalog_path = ROOT / f'Data/Maps/Imported/{AREA}/{AREA}.mapassets'
    catalog_bytes = catalog_path.read_bytes()
    placements_path = ROOT / f'Data/Maps/Authoring/{AREA}/{AREA}.mapplacements'
    placement_bytes = placements_path.read_bytes()
    placements = [[a or b for a, b in re.findall(r'"([^"]*)"|(\S+)', line)]
                  for line in placement_bytes.decode('utf-8-sig').splitlines()[1:] if line.strip()]
    catalog = {}
    for line in catalog_bytes.decode('utf-8-sig').splitlines():
        fields = [a or b for a, b in re.findall(r'"([^"]*)"|(\S+)', line)]
        if fields and fields[0].startswith('MAP_'):
            catalog[fields[0]] = fields[2]
    package = landscape.load_package(args.package, 'LV_OCN_EVENTIS_MHP_LAND01', landscape.UE3.LOSTARK_KR_AES_KEY)
    proxy, components, _ = landscape.parse_landscape_package(package)
    collisions = {(c.section_base_x, c.section_base_y): c
                  for c in landscape.parse_landscape_collision_components(package, proxy)}
    by_asset = {landscape.stable_asset_id(c)[1]: c for c in components}
    selected = {r['assetId'] for r in report['components']}
    assert len(selected) == len(report['components']) == 16
    assert selected == {a for a in catalog if '_LAND01_LC_' in a}
    converter = ROOT/'Tools/ModelAssetConverter/Bin/ModelAssetConverter.exe'
    output.mkdir(parents=True, exist_ok=True)
    staged = []
    for row in report['components']:
        asset = row['assetId']
        contract = row['metadata']['sourceLayerContract']
        assert re.fullmatch('[0-9a-f]{64}', contract['shaderMapKey'])
        assert contract['uvScale'] == .1
        component = by_asset[asset]
        placement, = [p for p in placements if p[4] == asset]
        anchor = landscape.component_world_anchor(component, proxy)
        assert max(abs(float(placement[5+k])-anchor[k]) for k in range(3)) < 1e-5
        assert list(map(float, placement[8:15])) == [0, 0, 0, 1, 1, 1, 1]
        assert placement[15] == '1'
        component.height_texture = landscape.decode_texture(package, component.heightmap_ref)
        component.weight_textures = [landscape.decode_texture(package, ref) for ref in component.weightmap_refs]
        source_collision = landscape.validate_collision_height_contract(component,
            collisions[(component.section_base_x, component.section_base_y)])
        target = (RESOURCE/catalog[asset]).resolve()
        assert target.is_relative_to((RESOURCE/'Map/LV_OCN_EVENTIS_MHP_LAND/Landscape').resolve())
        before = target.read_bytes()
        inputs = []
        for name in ('baked_diffuse.png', 'baked_normal.png'):
            path = target.parent/'textures'/name
            raw = path.read_bytes()
            assert digest(raw) == hashes[str(path.resolve()).casefold()], f'changed bake: {path}'
            inputs.append((path, raw))
        folder = output/asset
        folder.mkdir(exist_ok=True)
        gltf = folder/(asset+'.gltf')
        mesh = landscape.write_component_gltf(component, proxy, gltf, asset,
            dict(tiling=1, rotation=0), source_painted_surface=True)
        candidate = folder/(asset+'.wmodel')
        # The converter's narrow Assimp API cannot open a Korean absolute path.
        relative = lambda p: p.relative_to(ROOT).as_posix()
        command = [str(converter), relative(gltf), '-o', relative(candidate), '--pretransform',
                   '--no-auto-textures', '--scale', '100', '--material-remap',
                   f'{landscape.MATERIAL_NAME}={relative(inputs[0][0])}', '--normal-remap',
                   f'{landscape.MATERIAL_NAME}={relative(inputs[1][0])}']
        run = subprocess.run(command, cwd=ROOT, capture_output=True)
        (folder/'cook.log').write_bytes(run.stdout+run.stderr)
        assert run.returncode == 0, f'cook failed: {asset}'
        payload = candidate.read_bytes()
        cooked_source = validate_source_geometry(payload, component, proxy)
        if not args.restore_coordinate_basis:
            assert triangles(before) == triangles(payload), (
                f'geometry changed: {asset}; source basis repair requires --restore-coordinate-basis')
        for path, raw in inputs:
            assert (folder/'textures'/path.name).read_bytes() == raw
        assert all(('textures/'+name).encode('utf-16-le') in payload
                   for name in ('baked_diffuse.png', 'baked_normal.png'))
        staged.append((target, before, payload, inputs, dict(assetId=asset,
            modelAssetId=catalog[asset], beforeSha256=digest(before), afterSha256=digest(payload),
            shaderMapKey=contract['shaderMapKey'], geometry=mesh, cookedSource=cooked_source,
            sourceCollision=source_collision)))
        print('VALIDATED', asset, 'triangles', mesh['triangleCount'], flush=True)
    assert catalog_path.read_bytes() == catalog_bytes
    assert placements_path.read_bytes() == placement_bytes
    edges = landscape.validate_component_edges([by_asset[a] for a in selected])
    assert edges['mismatchCount'] == 0, 'source component seam mismatch'
    for target, before, _, inputs, _ in staged:
        assert target.read_bytes() == before
        assert all(p.read_bytes() == b for p, b in inputs)
    changed = []
    if args.apply:
        backup = output/'before'
        backup.mkdir(exist_ok=True)
        for target, before, _, _, _ in staged:
            saved = backup/target.name
            if saved.exists():
                assert saved.read_bytes() == before, 'backup conflict'
            else:
                saved.write_bytes(before)
        try:
            for target, before, payload, inputs, _ in staged:
                assert target.read_bytes() == before
                assert all(p.read_bytes() == b for p, b in inputs)
                temp = target.with_suffix('.wmodel.painted-tmp')
                assert not temp.exists()
                temp.write_bytes(payload)
                os.replace(temp, target)
                changed.append((target, before))
        except BaseException:
            for target, before in reversed(changed):
                target.write_bytes(before)
            raise
    (output/'receipt.json').write_text(json.dumps(dict(installed=len(changed),
        sourcePackageSha256=landscape.sha256_file(args.package),
        scope=('source-validated coordinate basis repair' if args.restore_coordinate_basis
               else 'painted surface UV/material only; HDR and source lighting remain separate'),
        componentEdges=edges,
        rows=[x[4] for x in staged]), indent=2), encoding='utf8')
    print('INSTALLED', len(changed), 'of', len(staged), flush=True)


if __name__ == '__main__':
    main()
