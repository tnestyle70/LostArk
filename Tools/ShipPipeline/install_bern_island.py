"""Installs the small Maharaka island (v2) and the Bern ocean gap plane into the Bern Area sources.

Adds (idempotent gap plane, LF/CRLF preserved per file):
  Data/Maps/Imported/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE_ISLAND00.mapassets      landscape + prop assets
  Data/Maps/Imported/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE_ISLAND00.mapplacements  (empty baseline)
  Data/Maps/Imported/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.mapset                  one shard row
  Data/Maps/Authoring/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.mapplacements          editor rows (tiles, props, pier, palms)
  Data/Maps/Authoring/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.mapmaterials.json      the props' materials + placementLighting

Default is a dry run; pass --apply to write (compare-and-swap on the current bytes, backups first).
--remove takes the island back out (the ocean gap plane stays).
Publish afterwards with Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_BER_BERNCASTLE -Scope Placements.
"""
import argparse
import hashlib
import json
import os
import shlex
import shutil
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import bern_island_layout as island  # noqa: E402

ROOT = island.ROOT
IMPORTED = os.path.join(ROOT, 'Data', 'Maps', 'Imported', island.BERN_AREA)
AUTH_DIR = os.path.join(ROOT, 'Data', 'Maps', 'Authoring', island.BERN_AREA)
AUTHORING = os.path.join(AUTH_DIR, island.BERN_AREA + '.mapplacements')
MATERIALS = os.path.join(AUTH_DIR, island.BERN_AREA + '.mapmaterials.json')
MAPSET = os.path.join(IMPORTED, island.BERN_AREA + '.mapset')
BACKUP = r'C:\Users\USER\.claude\jobs\46aea322\tmp\bern_backup'

# Ocean gap: WATER02_512 planes cover x -716..103 and 198..1836 over z -180..435; nothing sits between.
GAP_ASSET = 'MAP_7BA4AC84CD3A_LV_MODULE_WATER02_512'
GAP_SOURCE = island.PLACEMENT_LEVEL + ':water:gap-plane'
GAP_X = (102.0, 199.0)
GAP_Z = (-181.0, 436.0)
GAP_Y = 10.7142664
PLANE_UNITS = 5.12

# Entrance: a wrecked-ship deck used as a pier on the west (harbour) side, two palms as gate posts.
PIER_SUFFIX = 'LV_OCN_PEYTO_SHIPFLOOR01_SM_OVR_BFA30D74D6D7'
PIER_SCALE = 0.3
PIER_X_RANGE = (-26.9, 3.2)     # model extents at scale 1 (m)
PIER_Z_RANGE = (-11.7, 3.5)
PIER_TOP_Y = 0.2
PIER_TOP_ABOVE_SEA = 0.6
PALM_SUFFIX = 'BG_RHD_TREE_PALMTREE01_SM_KSY_OVR_F52718A2D5E1'
PALM_SCALE = 0.5


def gap_row():
    cx, cz = (GAP_X[0] + GAP_X[1]) / 2.0, (GAP_Z[0] + GAP_Z[1]) / 2.0
    sx = (GAP_X[1] - GAP_X[0]) / PLANE_UNITS
    sz = (GAP_Z[1] - GAP_Z[0]) / PLANE_UNITS
    return '%d "%s" "%s" "editor" "%s" %s %s %s 0 0 0 1 %s 60 %s 1' % (
        island.placement_id(GAP_SOURCE), GAP_SOURCE, island.PLACEMENT_LEVEL, GAP_ASSET,
        repr(cx), repr(GAP_Y), repr(cz), repr(round(sx, 6)), repr(round(sz, 6)))


def find_asset(assets, suffix):
    matches = [a for a in assets if a.endswith(suffix)]
    if len(matches) != 1:
        raise SystemExit('asset %s: %d matches' % (suffix, len(matches)))
    return matches[0]


def land_edge_west(ground, pivot):
    """Bern x of the westmost beach cell on the island centre row (ground above the waterline)."""
    z = pivot[1]
    x = pivot[0]
    edge = None
    while x > -6.0:
        h = ground.height(x, z)
        if h is not None and h > island.WATERLINE_ORIGINAL_Y + 0.05:
            edge = x
        x -= 1.0
        if edge is not None and (ground.height(x, z) is None or ground.height(x, z) < island.WATERLINE_ORIGINAL_Y - 0.3):
            break
    if edge is None:
        raise SystemExit('no beach found on the centre row')
    return island.bern_xz(edge, z, pivot)[0]


def entrance(ground, pivot):
    """Pier / gate / ship stop geometry in Bern metres (west side of the island)."""
    edge_x = land_edge_west(ground, pivot)
    center_z = island.ISLAND_CENTER[1]
    pier_len = (PIER_X_RANGE[1] - PIER_X_RANGE[0]) * PIER_SCALE
    origin_x = edge_x + 0.5 - PIER_X_RANGE[1] * PIER_SCALE     # east end of the deck sits 0.5 m inside the beach
    east_end = origin_x + PIER_X_RANGE[1] * PIER_SCALE
    tip_x = origin_x + PIER_X_RANGE[0] * PIER_SCALE
    z_mid = (PIER_Z_RANGE[0] + PIER_Z_RANGE[1]) / 2.0 * PIER_SCALE
    origin_z = center_z - z_mid
    origin_y = island.SEA_SURFACE_Y + PIER_TOP_ABOVE_SEA - PIER_TOP_Y * PIER_SCALE
    return {
        'landEdgeX': edge_x, 'pierOrigin': (origin_x, origin_y, origin_z), 'pierEastX': east_end, 'pierTipX': tip_x,
        'pierLengthM': pier_len, 'centerZ': center_z, 'deckY': island.SEA_SURFACE_Y + PIER_TOP_ABOVE_SEA,
        'stop': (tip_x - 3.0, center_z),
    }


def entrance_rows(assets, geometry):
    pier = find_asset(assets, PIER_SUFFIX)
    palm = find_asset(assets, PALM_SUFFIX)
    rows = []
    source = island.PLACEMENT_LEVEL + ':entrance:pier'
    rows.append((source, pier, island.format_row(source, pier, geometry['pierOrigin'], ('0', '0', '0', '1'),
                                                  (PIER_SCALE, PIER_SCALE, PIER_SCALE)), None))
    for index, dz in enumerate((-1.6, 1.6)):
        source = island.PLACEMENT_LEVEL + ':entrance:palm%d' % index
        position = (geometry['pierTipX'] + 1.0, geometry['deckY'], geometry['centerZ'] + dz)
        rows.append((source, palm, island.format_row(source, palm, position, ('0', '0', '0', '1'),
                                                      (PALM_SCALE, PALM_SCALE, PALM_SCALE)), None))
    return rows


def read_lines(path):
    raw = open(path, 'rb').read()
    if b'\r' in raw:
        raise SystemExit('unexpected CR in ' + path)
    return raw, raw.decode('utf-8').split('\n')


def atomic_write(path, data, expected_sha):
    if hashlib.sha256(open(path, 'rb').read()).hexdigest() != expected_sha:
        raise SystemExit('file changed during install: ' + path)
    temporary = path + '.island.tmp'
    with open(temporary, 'wb') as handle:
        handle.write(data)
    os.replace(temporary, path)


def build_plan():
    assets = island.load_catalog()
    tiles = island.load_tiles()
    pivot = island.offset(tiles)
    ground = island.GroundSampler(tiles)
    geometry = entrance(ground, pivot)
    rows = island.tile_rows(tiles, pivot) + island.prop_rows(pivot, ground, assets) + entrance_rows(assets, geometry)
    used = []
    for _source, asset, _row, _orig in rows:
        if asset not in used:
            used.append(asset)
    existing = bern_existing_assets()
    shared = [a for a in used if a in existing]
    return {'assets': assets, 'tiles': tiles, 'pivot': pivot, 'geometry': geometry, 'rows': rows, 'used': used,
            'shared': shared, 'new': [a for a in used if a not in existing]}


def bern_existing_assets():
    """Asset IDs already declared by any other Bern shard catalog (the ID is area-global)."""
    found = set()
    for name in os.listdir(IMPORTED):
        if name.endswith('.mapassets') and name != island.SHARD_CATALOG:
            for line in open(os.path.join(IMPORTED, name), 'rb').read().decode('utf-8').split(chr(10))[1:]:
                if line.strip():
                    found.add(shlex.split(line)[0])
    return found


def material_payload(plan):
    document = json.load(open(island.MHP_MATERIALS, encoding='utf-8'))
    used = set(plan['new'])
    materials = [m for m in document['materials'] if m['assetId'] in used]
    lighting = document.get('placementLighting', [])
    by_source = {entry['sourcePlacementId']: entry for entry in lighting}
    by_asset = {}
    for entry in lighting:
        by_asset.setdefault(entry['assetId'], entry)
    baked = {m['assetId'] for m in materials if m.get('bakedLighting')}
    bern_doc = json.load(open(MATERIALS, encoding='utf-8'))
    shared = set(plan['shared'])
    baked |= {m['assetId'] for m in bern_doc['materials'] if m['assetId'] in shared and m.get('bakedLighting')}
    new_lighting = []
    missing = []
    for source, asset, _row, original in plan['rows']:
        entry = by_source.get(original) if original else None
        if asset not in baked:
            continue
        if entry is None and asset in baked:
            entry = by_asset.get(asset)      # pier / palms: reuse the asset's first baked-lighting record
        if entry is not None and entry['assetId'] == asset:
            copy = dict(entry)
            copy['sourcePlacementId'] = source
            new_lighting.append(copy)
        else:
            missing.append((source, asset))
    return materials, new_lighting, missing


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--apply', action='store_true')
    parser.add_argument('--remove', action='store_true')
    parser.add_argument('--dock-json', default='')
    args = parser.parse_args()

    plan = build_plan()
    geometry = plan['geometry']
    materials, lighting, missing = material_payload(plan)
    print('pivot %.3f %.3f  scale %.3f stretch %.2f placementY %.3f' % (
        plan['pivot'][0], plan['pivot'][1], island.SCALE, island.TERRAIN_STRETCH, island.PLACEMENT_Y))
    print('rows %d (tiles 16, props %d, entrance 3), assets %d (%d already in Bern), materials %d, lighting %d, baked-without-lighting %d' % (
        len(plan['rows']), len(plan['rows']) - 19, len(plan['used']), len(plan['shared']), len(materials), len(lighting), len(missing)))
    print('entrance', json.dumps({k: (list(v) if isinstance(v, tuple) else round(v, 3)) for k, v in geometry.items()}))
    for item in missing:
        print('  WARNING baked material without lighting row:', item)

    catalog_lines = ['LOSTARK_MAP_ASSET_CATALOG 4 "%s" %d' % (island.BERN_AREA, len(plan['new']))] + [island.tinted_catalog_line(a, plan['assets'][a]) for a in plan['new']]
    catalog_text = '\n'.join(catalog_lines) + '\n'
    placements_text = 'LOSTARK_MAP_PLACEMENTS 2 "%s" 0\n' % island.BERN_AREA
    catalog_path = os.path.join(IMPORTED, island.SHARD_CATALOG)
    placements_path = os.path.join(IMPORTED, island.SHARD_PLACEMENTS)

    set_raw, set_lines = read_lines(MAPSET)
    if set_lines and set_lines[-1] == '':
        set_lines = set_lines[:-1]
    shard_prefix = '"%s" ' % island.SHARD_ID
    has_shard = any(line.startswith(shard_prefix) for line in set_lines[1:])
    new_set = [line for line in set_lines if not (line.startswith(shard_prefix))] if args.remove else list(set_lines)
    if args.remove:
        new_set[0] = 'LOSTARK_MAP_SHARD_SET 1 "%s" %d' % (island.BERN_AREA, len(new_set))
    elif not has_shard:
        new_set[0] = 'LOSTARK_MAP_SHARD_SET 1 "%s" %d' % (island.BERN_AREA, len(set_lines))
        new_set.append('"%s" "%s" "%s" %d 0' % (island.SHARD_ID, island.SHARD_CATALOG, island.SHARD_PLACEMENTS, len(plan['new'])))
    else:
        raise SystemExit('island shard already present; run --remove first')
    new_set_text = '\n'.join(new_set) + '\n'

    auth_raw, auth_lines = read_lines(AUTHORING)
    trailing = auth_lines and auth_lines[-1] == ''
    body = auth_lines[:-1] if trailing else auth_lines
    header_count = int(body[0].split()[-1])
    if header_count != len(body) - 1:
        raise SystemExit('authoring header count mismatch')
    kept = [body[0]]
    removed = 0
    for line in body[1:]:
        parts = line.split('"')
        if len(parts) > 3 and parts[3] == island.PLACEMENT_LEVEL and parts[1] != GAP_SOURCE:
            removed += 1
            continue
        kept.append(line)
    existing_sources = {line.split('"')[1] for line in kept[1:] if line.count('"') > 2}
    add_rows = []
    if not args.remove:
        add_rows = [row for _s, _a, row, _o in plan['rows']]
        if GAP_SOURCE not in existing_sources:
            add_rows.append(gap_row())
    kept.extend(add_rows)
    kept[0] = 'LOSTARK_MAP_PLACEMENTS 2 "%s" %d' % (island.BERN_AREA, len(kept) - 1)
    new_auth_text = '\n'.join(kept) + '\n'
    print('authoring: removed %d old island rows, added %d, total %d' % (removed, len(add_rows), len(kept) - 1))

    mat_raw = open(MATERIALS, 'rb').read()
    mat_doc = json.loads(mat_raw.decode('utf-8'))
    shard_asset_ids = set(plan['new'])
    mat_doc['materials'] = [m for m in mat_doc['materials'] if m['assetId'] not in shard_asset_ids]
    mat_doc['placementLighting'] = [e for e in mat_doc.get('placementLighting', [])
                                    if not e['sourcePlacementId'].startswith(island.PLACEMENT_LEVEL + ':')]
    if not args.remove:
        mat_doc['materials'].extend(materials)
        mat_doc['placementLighting'].extend(lighting)
    new_mat_bytes = (json.dumps(mat_doc, indent=2, ensure_ascii=True).replace('\n', '\r\n') + '\r\n').encode('utf-8')

    if args.dock_json:
        payload = {
            'status': 'FINAL_V2',
            'coordinateSystem': 'Bern runtime metres (same as Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json placement positions)',
            'islandCenter': {'x': island.ISLAND_CENTER[0], 'y': island.SEA_SURFACE_Y, 'z': island.ISLAND_CENTER[1]},
            'pier': {'deckTopY': round(geometry['deckY'], 3), 'eastEndX': round(geometry['pierEastX'], 2),
                     'tipX': round(geometry['pierTipX'], 2), 'centerZ': round(geometry['centerZ'], 2),
                     'widthM': round((PIER_Z_RANGE[1] - PIER_Z_RANGE[0]) * PIER_SCALE, 2)},
            'gatePalms': [{'x': round(geometry['pierTipX'] + 1.0, 2), 'y': round(geometry['deckY'], 3),
                           'z': round(geometry['centerZ'] + dz, 2)} for dz in (-1.6, 1.6)],
            'shipStopPoint': {'x': round(geometry['stop'][0], 2), 'z': round(geometry['stop'][1], 2)},
            'note': 'pier and palms are visual props; the ship stops 3 m west of the pier tip; y of stop/trigger = BernSea level',
        }
        with open(args.dock_json, 'w', encoding='utf-8') as handle:
            json.dump(payload, handle, indent=2)
        print('wrote', args.dock_json)
    if not args.apply:
        print('dry run only; pass --apply to write')
        return 0

    os.makedirs(BACKUP, exist_ok=True)
    stamp = 'v2_remove' if args.remove else 'v2'
    shutil.copyfile(MAPSET, os.path.join(BACKUP, 'LV_BER_BERNCASTLE.mapset.before_' + stamp))
    shutil.copyfile(AUTHORING, os.path.join(BACKUP, 'LV_BER_BERNCASTLE.mapplacements.before_' + stamp))
    shutil.copyfile(MATERIALS, os.path.join(BACKUP, 'LV_BER_BERNCASTLE.mapmaterials.json.before_' + stamp))
    if args.remove:
        for path in (catalog_path, placements_path):
            if os.path.exists(path):
                os.remove(path)
    else:
        for path, text in ((catalog_path, catalog_text), (placements_path, placements_text)):
            with open(path, 'w', encoding='utf-8', newline='\n') as handle:
                handle.write(text)
    atomic_write(MAPSET, new_set_text.encode('utf-8'), hashlib.sha256(set_raw).hexdigest())
    atomic_write(AUTHORING, new_auth_text.encode('utf-8'), hashlib.sha256(auth_raw).hexdigest())
    atomic_write(MATERIALS, new_mat_bytes, hashlib.sha256(mat_raw).hexdigest())
    print('applied')
    return 0


if __name__ == '__main__':
    sys.exit(main())
