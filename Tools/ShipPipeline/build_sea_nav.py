"""Builds the Bern sea navigation region (LV_BER_BERNCASTLE.BernSea).

The original game sails in a separate ocean map; this project has no such level, so the ship
sails on a synthetic flat navigation region placed on the sea north and east of the Bern3
piers. The region is authoring data in the normal MapTool source format (navsource v2), so the
existing publisher, server and client consume it like any other detail region.

2026-09-29: the window grew from the 148 x 106 m harbour to about 460 x 494 m (0.5 m cells,
908,960 cells; the publisher cap is 1,000,000 cells per grid) so ships can sail to the
simplified Maharaka island (Tools/ShipPipeline/bern_island_layout.py). Keep-outs now also come
from the rendered terrain: the 42 Bern landscape tiles and the island tiles are rasterised and
any cell whose ground reaches the water line is closed, with a clearance ring.

Everything derived here is an ASSUMPTION that can be changed by editing the constants below and
running the script again (then publish the Area):

  SEA_SURFACE_Y  water surface height. Evidence: the harbour boats (COMMON_SHIP01) stand at
                 y 10.78..10.97 and the two ocean planes (LV_MODULE_WATER02_512) at y 10.71/10.92.
  SHIP_DRAFT_M   signed depth of the ship root below the surface. Negative lifts the root above
                 the water. Measured 2026-09-25: every ship WModel origin is its keel (lowest
                 vertex -0.17..-0.66 m at modelPreScale 0.01) and the Client adds each ship's
                 modelLiftMeters (VehicleCatalog.json) so the keel sits at the root. -0.15 keeps
                 the hull bottom 0.15 m above the water. The navigation level is
                 SEA_SURFACE_Y - SHIP_DRAFT_M because the ship root is drawn at the player's feet.
  CELL           grid cell size in metres (0.5 since 2026-09-25; was 1.0).
  RECT           the window (x0, x1, z0, z1) that may hold sea; its outer 2 m ring is closed.
  CLEARANCE_M    keep-out around every dock/land navigation cell and the island terrain.

Nothing here edits the user-owned Bern/Bern2/Bern3 grids; they are only read.
"""
import argparse
import json
import math
import os
import re
import sys

import cv2
import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import bern_island_layout as island  # noqa: E402
import bern_terrain as terrain  # noqa: E402
import bern_isl72 as isl72  # noqa: E402

ROOT = island.ROOT
NAVDIR = os.path.join(ROOT, 'Data', 'Navigation')
AREA = island.BERN_AREA
REGION = 'BernSea'
GRID_ID = AREA + '.' + REGION
PLACEMENTS = os.path.join(ROOT, 'Data', 'Maps', 'Authoring', AREA, AREA + '.mapplacements')

SEA_SURFACE_Y = island.SEA_SURFACE_Y
SHIP_DRAFT_M = -0.15
RECT = (140.0, 600.0, -650.0, -156.0)
CELL = 0.5
CLEARANCE_M = 6.0
BERN_TERRAIN_CLEARANCE_M = 3.0
TERRAIN_WATERLINE_MARGIN_M = 0.3
EDGE_MARGIN_M = 2.0
MAX_CELLS = 1000000
HARBOUR_RETURN = (300.0, -235.0)
DOCK_DISTANCE_M = (6.0, 14.0)
STATIC_KEEP_OUT = (
    (re.compile(r'ESTOCSHIP|COMMON_SHIP0[14]'), 9.0),
    (re.compile(r'ROCK|CLIFF|PIER|DOCK|MOOR|BUOY|CRANE|BRIDGE|PORTPROP|STELE'), 5.0),
)


class Source:
    def __init__(self, region):
        path = os.path.join(NAVDIR, '%s.%s.navsource' % (AREA, region))
        with open(path, 'r', encoding='utf-8') as handle:
            head = handle.readline().split()
            self.w, self.h = int(head[3]), int(head[4])
            self.cell, self.ox, self.oz = float(head[5]), float(head[6]), float(head[7])
            self.surface = bytearray(self.w * self.h)
            for line in handle:
                token = line.split()
                if len(token) == 5:
                    self.surface[int(token[1]) * self.w + int(token[0])] = int(token[2])

    def cells(self):
        for z in range(self.h):
            base = z * self.w
            for x in range(self.w):
                if self.surface[base + x]:
                    yield (self.ox + (x + 0.5) * self.cell, self.oz + (z + 0.5) * self.cell)


def read_static_keep_outs(rect):
    out = []
    if not os.path.exists(PLACEMENTS):
        return out
    pattern = re.compile(r'\d+ "[^"]*" "[^"]*" "\w+" "([^"]+)" (.*)')
    with open(PLACEMENTS, 'rb') as handle:
        for raw in handle:
            match = pattern.match(raw.decode('utf-8', 'replace'))
            if not match:
                continue
            name = match.group(1).upper()
            radius = 0.0
            for regex, value in STATIC_KEEP_OUT:
                if regex.search(name):
                    radius = value
                    break
            if radius <= 0.0:
                continue
            numbers = match.group(2).split()[:3]
            try:
                x, y, z = float(numbers[0]), float(numbers[1]), float(numbers[2])
            except (ValueError, IndexError):
                continue
            if 4.0 <= y <= 20.0 and rect[0] - 12 <= x <= rect[1] + 12 and rect[2] - 12 <= z <= rect[3] + 12:
                out.append((x, z, radius))
    return out


def read_bern_landscape_tiles(rect):
    """Bern landscape tiles whose 39.68 m footprint touches the window: [(modelPath, position)]."""
    catalog = {}
    with open(os.path.join(ROOT, 'Data', 'Maps', 'Imported', AREA, AREA + '_LANDSCAPE.mapassets'), 'rb') as handle:
        import shlex
        for line in handle.read().decode('utf-8').split('\n')[1:]:
            if line.strip():
                tokens = shlex.split(line)
                catalog[tokens[0]] = tokens[2]
    pattern = re.compile(r'^\d+ "([^"]*:landscape:[^"]*)" "[^"]*" "\w+" "([^"]+)" (.*)$')
    tiles = []
    with open(PLACEMENTS, 'rb') as handle:
        for line in handle.read().decode('utf-8', 'replace').split('\n')[1:]:
            match = pattern.match(line)
            if not match or match.group(2) not in catalog:
                continue
            numbers = match.group(3).split()
            px, pz = float(numbers[0]), float(numbers[2])
            if px + 40.0 >= rect[0] and px <= rect[1] and pz + 40.0 >= rect[2] and pz <= rect[3]:
                tiles.append((os.path.join(island.RESOURCES, catalog[match.group(2)]), (px, float(numbers[1]), pz)))
    return tiles


def circle_mask(shape, x0, z0, cell, discs):
    mask = np.zeros(shape, dtype=np.uint8)
    for (cx, cz, radius) in discs:
        gx = int(math.floor((cx - x0) / cell))
        gz = int(math.floor((cz - z0) / cell))
        cv2.circle(mask, (gx, gz), max(int(round(radius / cell)), 1), 1, -1)
    return mask


def dilate_mask(obstacle, cell, clearance):
    """1 where within clearance metres of an obstacle cell (Euclidean)."""
    free = (obstacle == 0).astype(np.uint8)
    distance = cv2.distanceTransform(free, cv2.DIST_L2, 5)
    return (distance * cell <= clearance).astype(np.uint8), distance * cell


def build(rect, cell, clearance, sea_y, draft):
    x0, x1, z0, z1 = rect
    width = int(round((x1 - x0) / cell))
    height = int(round((z1 - z0) / cell))
    if width * height > MAX_CELLS:
        raise SystemExit('window %dx%d exceeds the %d cell publisher cap' % (width, height, MAX_CELLS))
    shape = (height, width)

    # Existing navigation ground (docks, decks, land) of the user-owned Bern regions.
    ground = np.zeros(shape, dtype=np.uint8)
    for region in ('Bern', 'Bern3', 'Bern2'):
        source = Source(region)
        for (px, pz) in source.cells():
            gx = int(math.floor((px - x0) / cell))
            gz = int(math.floor((pz - z0) / cell))
            if 0 <= gx < width and 0 <= gz < height:
                ground[gz, gx] = 1
    blocked_ground, _ = dilate_mask(ground, cell, clearance)

    statics = read_static_keep_outs(rect)
    blocked_static = circle_mask(shape, x0, z0, cell, statics)

    # Rendered terrain: Bern landscapes and the island tiles, rasterised at the cell grid.
    height_bern = terrain.new_height_grid(width, height)
    bern_tiles = read_bern_landscape_tiles(rect)
    for path, position in bern_tiles:
        terrain.rasterise_max_height(height_bern, terrain.world_triangles(path, position), x0, z0, cell, width, height)
    waterline = sea_y - TERRAIN_WATERLINE_MARGIN_M
    bern_land = (height_bern > waterline).astype(np.uint8)
    blocked_bern, _ = dilate_mask(bern_land, cell, BERN_TERRAIN_CLEARANCE_M)

    # The retail ocean island prop (EFDLProp_ISL_00072) placed by Tools/ShipPipeline/bern_isl72.py.
    height_island = terrain.new_height_grid(width, height)
    terrain.rasterise_max_height(height_island, isl72.world_triangles(), x0, z0, cell, width, height)
    island_land = (height_island > waterline).astype(np.uint8)
    blocked_island, _ = dilate_mask(island_land, cell, clearance)

    blocked = (blocked_ground | blocked_static | blocked_bern | blocked_island).astype(np.uint8)
    margin = int(math.ceil(EDGE_MARGIN_M / cell))
    blocked[:margin, :] = 1
    blocked[-margin:, :] = 1
    blocked[:, :margin] = 1
    blocked[:, -margin:] = 1

    # Largest 4-connected open component.
    count, labels = cv2.connectedComponents((blocked == 0).astype(np.uint8), connectivity=4)
    sizes = np.bincount(labels.ravel(), minlength=count)
    sizes[0] = 0
    best_id = int(np.argmax(sizes)) if count > 1 else 0
    best_size = int(sizes[best_id]) if best_id else 0
    walk = (labels == best_id).astype(np.uint8) if best_id else np.zeros(shape, dtype=np.uint8)
    open_components = int(np.count_nonzero(sizes))
    return {
        'width': width, 'height': height, 'walk': walk, 'statics': len(statics), 'best': best_size,
        'components': open_components, 'island_land': island_land, 'island_shift': isl72.ISLAND_ORIGIN,
        'bern_tiles': len(bern_tiles), 'bern_land_cells': int(bern_land.sum()),
        'island_land_cells': int(island_land.sum()),
    }


def pick_dock(result, rect, cell, level, target=None):
    """Ship stop cell on the harbour side of the island plus the harbour return cell."""
    x0, _x1, z0, _z1 = rect
    walk = result['walk']
    free = (result['island_land'] == 0).astype(np.uint8)
    distance = cv2.distanceTransform(free, cv2.DIST_L2, 5) * cell
    lo, hi = DOCK_DISTANCE_M
    candidates = np.argwhere((walk == 1) & (distance >= lo) & (distance <= hi))
    if candidates.size == 0:
        return None
    gx = candidates[:, 1]
    gz = candidates[:, 0]
    px = x0 + (gx + 0.5) * cell
    pz = z0 + (gz + 0.5) * cell
    goal = target if target is not None else HARBOUR_RETURN
    best = int(np.argmin((px - goal[0]) ** 2 + (pz - goal[1]) ** 2))
    stop = (float(px[best]), float(pz[best]))
    ys, xs = np.nonzero(result['island_land'])
    center = (x0 + (float(xs.mean()) + 0.5) * cell, z0 + (float(ys.mean()) + 0.5) * cell)
    yaw = math.degrees(math.atan2(center[0] - stop[0], center[1] - stop[1]))
    # Harbour return: the walkable cell nearest to the desired point.
    walk_cells = np.argwhere(walk == 1)
    wx = x0 + (walk_cells[:, 1] + 0.5) * cell
    wz = z0 + (walk_cells[:, 0] + 0.5) * cell
    nearest = int(np.argmin((wx - HARBOUR_RETURN[0]) ** 2 + (wz - HARBOUR_RETURN[1]) ** 2))
    harbour = (float(wx[nearest]), float(wz[nearest]))
    return {
        'islandCenter': {'x': round(center[0], 2), 'y': SEA_SURFACE_Y, 'z': round(center[1], 2)},
        'shipStopPoint': {'x': round(stop[0], 2), 'y': level, 'z': round(stop[1], 2)},
        'retailAnchor': {'x': round(isl72.ANCHOR_BERN[0], 2), 'y': level, 'z': round(isl72.ANCHOR_BERN[1], 2)},
        'dockTrigger': {'x': round(isl72.ANCHOR_BERN[0], 2), 'y': level, 'z': round(isl72.ANCHOR_BERN[1], 2),
                        'halfExtents': [8.0, 4.0, 8.0], 'yawDegrees': round(yaw, 1)},
        'harbourReturnSpawn': {'x': round(harbour[0], 2), 'y': level, 'z': round(harbour[1], 2), 'yawDegrees': 0.0,
                               'note': 'open sea cell nearest the Bern3 pier structure, same level as BernSea; a returning ship appears here'},
        'distanceToIslandLandM': round(float(distance[int(round((stop[1] - z0) / cell - 0.5)), int(round((stop[0] - x0) / cell - 0.5))]), 2),
    }


def write_source(width, height, walk, rect, cell, level):
    x0, x1, z0, z1 = rect
    path = os.path.join(NAVDIR, GRID_ID + '.navsource')
    total = width * height
    header = ['LOSTARK_NAVGRID_SOURCE', '2', '"%s"' % GRID_ID, str(width), str(height),
              repr(cell), repr(x0), repr(z0),
              repr((x0 + x1) / 2.0), repr(level - 6.0), repr((z0 + z1) / 2.0),
              repr(x1 - x0), '30', repr(z1 - z0), '0', '50', '1', str(total)]
    flat = walk.ravel()
    walk_line = '1 1 %s' % repr(level)
    lines = [' '.join(header)]
    lines.extend(('%d %d %s' % (index % width, index // width, walk_line)) if flat[index] else
                 ('%d %d 0 0 0' % (index % width, index // width)) for index in range(total))
    with open(path, 'w', encoding='utf-8', newline='\n') as handle:
        handle.write('\n'.join(lines) + '\n')
    return path


def preview(width, height, walk, step=12):
    rows = []
    for gz in range(0, height, step):
        row = ''
        for gx in range(0, width, step):
            row += '~' if walk[gz, gx] else '#'
        rows.append(row)
    return rows


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--write', action='store_true', help='write the navsource (default: dry run)')
    parser.add_argument('--sea-y', type=float, default=SEA_SURFACE_Y)
    parser.add_argument('--draft', type=float, default=SHIP_DRAFT_M)
    parser.add_argument('--clearance', type=float, default=CLEARANCE_M)
    parser.add_argument('--dock-json', default='', help='write the island dock hand-off JSON')
    args = parser.parse_args()
    level = round(args.sea_y - args.draft, 4)
    result = build(RECT, CELL, args.clearance, args.sea_y, args.draft)
    width, height, walk = result['width'], result['height'], result['walk']
    print('grid %dx%d cell %.1f level %.2f static keep-outs %d bern landscape tiles %d (land cells %d) island land cells %d '
          'open components %d largest %d cells (%.0f m2)' % (
              width, height, CELL, level, result['statics'], result['bern_tiles'], result['bern_land_cells'],
              result['island_land_cells'], result['components'], result['best'], result['best'] * CELL * CELL))
    print('island origin x %.3f z %.3f scale %.2f retail anchor %.2f %.2f' % (
        isl72.ISLAND_ORIGIN[0], isl72.ISLAND_ORIGIN[1], isl72.ISLAND_SCALE, isl72.ANCHOR_BERN[0], isl72.ANCHOR_BERN[1]))
    for row in preview(width, height, walk):
        print(row)
    dock = pick_dock(result, RECT, CELL, level, isl72.ANCHOR_BERN)
    print('dock', json.dumps(dock))
    if args.dock_json and dock:
        payload = {
            'status': 'FINAL from Tools/ShipPipeline/build_sea_nav.py',
            'coordinateSystem': 'Bern runtime metres (same as Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json placement positions)',
            'seaNavigation': {'region': GRID_ID, 'level_y': level, 'cellMetres': CELL,
                              'window': {'xMin': RECT[0], 'xMax': RECT[1], 'zMin': RECT[2], 'zMax': RECT[3]}},
        }
        payload.update({k: v for k, v in dock.items() if k != 'distanceToIslandLandM'})
        with open(args.dock_json, 'w', encoding='utf-8') as handle:
            json.dump(payload, handle, indent=2)
        print('wrote', args.dock_json)
    if result['best'] < 1500:
        print('refusing to write: sea region too small', file=sys.stderr)
        return 2
    if dock is None:
        print('refusing to write: no dock cell found', file=sys.stderr)
        return 3
    if args.write:
        print('wrote', write_source(width, height, walk, RECT, CELL, level))
    return 0


if __name__ == '__main__':
    sys.exit(main())
