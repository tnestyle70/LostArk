"""Builds the Bern harbour sea navigation region (LV_BER_BERNCASTLE.BernSea).

The original game sails in a separate ocean map; this project has no such level, so the ship
sails on a synthetic flat navigation region placed on the harbour water in front of the Bern3
piers. The region is authoring data in the normal MapTool source format (navsource v2), so the
existing publisher, server and client consume it like any other detail region.

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
  RECT           the harbour window (x0, x1, z0, z1) that may hold sea.
  CLEARANCE_M    keep-out around every dock/land navigation cell and every static ship/rock.

Nothing here edits the user-owned Bern/Bern2/Bern3 grids; they are only read.
"""
import argparse
import collections
import math
import os
import re
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
NAVDIR = os.path.join(ROOT, 'Data', 'Navigation')
AREA = 'LV_BER_BERNCASTLE'
REGION = 'BernSea'
GRID_ID = AREA + '.' + REGION
PLACEMENTS = os.path.join(ROOT, 'Data', 'Maps', 'Authoring', AREA, AREA + '.mapplacements')

SEA_SURFACE_Y = 10.8
SHIP_DRAFT_M = -0.15
RECT = (192.0, 340.0, -262.0, -156.0)
CELL = 0.5
CLEARANCE_M = 6.0
EDGE_MARGIN_M = 2.0
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


def build(rect, cell, clearance, sea_y, draft):
    x0, x1, z0, z1 = rect
    width = int(round((x1 - x0) / cell))
    height = int(round((z1 - z0) / cell))
    blocked = bytearray(width * height)

    def index(px, pz):
        return int((pz - z0) // cell) * width + int((px - x0) // cell)

    def block_disc(cx, cz, radius):
        lo_x = int((cx - radius - x0) // cell)
        hi_x = int((cx + radius - x0) // cell)
        lo_z = int((cz - radius - z0) // cell)
        hi_z = int((cz + radius - z0) // cell)
        for gz in range(max(lo_z, 0), min(hi_z, height - 1) + 1):
            for gx in range(max(lo_x, 0), min(hi_x, width - 1) + 1):
                px = x0 + (gx + 0.5) * cell
                pz = z0 + (gz + 0.5) * cell
                if (px - cx) ** 2 + (pz - cz) ** 2 <= radius * radius:
                    blocked[gz * width + gx] = 1

    # Existing navigation ground (docks, decks, land) of the user-owned Bern regions.
    for region in ('Bern', 'Bern3', 'Bern2'):
        source = Source(region)
        for (px, pz) in source.cells():
            if x0 - clearance <= px <= x1 + clearance and z0 - clearance <= pz <= z1 + clearance:
                block_disc(px, pz, clearance)
    statics = read_static_keep_outs(rect)
    for (px, pz, radius) in statics:
        block_disc(px, pz, radius)
    # Rectangle edge margin.
    margin = int(math.ceil(EDGE_MARGIN_M / cell))
    for gz in range(height):
        for gx in range(width):
            if gx < margin or gz < margin or gx >= width - margin or gz >= height - margin:
                blocked[gz * width + gx] = 1

    # Largest 4-connected open component.
    label = [0] * (width * height)
    best_id, best_size, next_id = 0, 0, 0
    sizes = {}
    for start in range(width * height):
        if blocked[start] or label[start]:
            continue
        next_id += 1
        stack = [start]
        label[start] = next_id
        count = 0
        while stack:
            current = stack.pop()
            count += 1
            cx, cz = current % width, current // width
            for nx, nz in ((cx + 1, cz), (cx - 1, cz), (cx, cz + 1), (cx, cz - 1)):
                if 0 <= nx < width and 0 <= nz < height:
                    neighbour = nz * width + nx
                    if not blocked[neighbour] and not label[neighbour]:
                        label[neighbour] = next_id
                        stack.append(neighbour)
        sizes[next_id] = count
        if count > best_size:
            best_id, best_size = next_id, count
    walk = bytearray(width * height)
    for i in range(width * height):
        if label[i] == best_id and best_id:
            walk[i] = 1
    return width, height, walk, len(statics), best_size, sizes


def write_source(width, height, walk, rect, cell, level):
    x0, x1, z0, z1 = rect
    path = os.path.join(NAVDIR, GRID_ID + '.navsource')
    total = width * height
    header = ['LOSTARK_NAVGRID_SOURCE', '2', '"%s"' % GRID_ID, str(width), str(height),
              repr(cell), repr(x0), repr(z0),
              repr((x0 + x1) / 2.0), repr(level - 6.0), repr((z0 + z1) / 2.0),
              repr(x1 - x0), '30', repr(z1 - z0), '0', '50', '1', str(total)]
    lines = [' '.join(header)]
    for gz in range(height):
        for gx in range(width):
            if walk[gz * width + gx]:
                lines.append('%d %d 1 1 %s' % (gx, gz, repr(level)))
            else:
                lines.append('%d %d 0 0 0' % (gx, gz))
    with open(path, 'w', encoding='utf-8', newline='\n') as handle:
        handle.write('\n'.join(lines) + '\n')
    return path


def preview(width, height, walk, step=3):
    rows = []
    for gz in range(0, height, step):
        row = ''
        for gx in range(0, width, step):
            row += '~' if walk[gz * width + gx] else '#'
        rows.append(row)
    return rows


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--write', action='store_true', help='write the navsource (default: dry run)')
    parser.add_argument('--sea-y', type=float, default=SEA_SURFACE_Y)
    parser.add_argument('--draft', type=float, default=SHIP_DRAFT_M)
    parser.add_argument('--clearance', type=float, default=CLEARANCE_M)
    args = parser.parse_args()
    level = round(args.sea_y - args.draft, 4)
    width, height, walk, statics, best, sizes = build(RECT, CELL, args.clearance, args.sea_y, args.draft)
    print('grid %dx%d cell %.1f level %.2f static keep-outs %d open components %d largest %d cells (%.0f m2)' % (
        width, height, CELL, level, statics, len(sizes), best, best * CELL * CELL))
    for row in preview(width, height, walk):
        print(row)
    if best < 1500:
        print('refusing to write: sea region too small', file=sys.stderr)
        return 2
    if args.write:
        print('wrote', write_source(width, height, walk, RECT, CELL, level))
    return 0


if __name__ == '__main__':
    sys.exit(main())
