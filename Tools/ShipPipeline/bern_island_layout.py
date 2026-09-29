"""Single source of truth for the small Maharaka island placed in the Bern sea (v2, 2026-09-29).

Appearance only. The original far island is the Maharaka level itself seen at a tiny scale (the ocean
world LV_OCN_World* sublevels hold only rocks/cliffs near it, no separate model), so the island is the
installed Maharaka landscape (16 tiles) plus the pool / slide / wrecked-ship / parasol / palm props,
copied with one uniform scale SCALE. The landscape tiles are stretched vertically by a further factor
so the flat tile skirt stays hidden under the sea; props follow the stretched ground. Ships reach it
through the BernSea navigation region; entering Maharaka is a separate Server trigger
(Gameplay.world.json).

Every number the sea builder and the placement installer share lives here so the navigation keep-out
and the drawn terrain cannot drift apart.
"""
import hashlib
import os
import re
import shlex

import numpy as np

from bern_terrain import MODEL_SCALE, read_static_mesh

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
BERN_AREA = 'LV_BER_BERNCASTLE'
MHP_AREA = 'LV_OCN_EVENTIS_MHP'
MHP_CATALOG = os.path.join(ROOT, 'Data', 'Maps', 'Imported', MHP_AREA, MHP_AREA + '.mapassets')
MHP_PLACEMENTS = os.path.join(ROOT, 'Data', 'Maps', 'Authoring', MHP_AREA, MHP_AREA + '.mapplacements')
MHP_MATERIALS = os.path.join(ROOT, 'Data', 'Maps', 'Authoring', MHP_AREA, MHP_AREA + '.mapmaterials.json')
RESOURCES = os.path.join(ROOT, 'Client', 'Bin', 'Resources')

SHARD_ID = 'ISLAND00'
SHARD_CATALOG = BERN_AREA + '_' + SHARD_ID + '.mapassets'
SHARD_PLACEMENTS = BERN_AREA + '_' + SHARD_ID + '.mapplacements'
PLACEMENT_LEVEL = BERN_AREA + '_ISLAND'

SEA_SURFACE_Y = 10.8
# Uniform scale of the whole island. The original far island is about 16 x 21 m; the Maharaka blob
# (111 x 100 m) at 0.22 is about 24 x 22 m including the props.
SCALE = 0.22
# Extra vertical stretch of the landscape tiles only (the skirt is a flat plate 4.67 m under the beach).
TERRAIN_STRETCH = 3.5
VERTICAL_SCALE = SCALE * TERRAIN_STRETCH
TERRAIN_SCALE = (SCALE, VERTICAL_SCALE, SCALE)
# Original beach waterline in Maharaka coordinates (m); maps onto the Bern sea surface.
WATERLINE_ORIGINAL_Y = 20.03
PLACEMENT_Y = SEA_SURFACE_Y - WATERLINE_ORIGINAL_Y * VERTICAL_SCALE
# Bern-space position of the visible sand blob centre (metres).
ISLAND_CENTER = (440.0, -480.0)

# v3: every visible Maharaka prop that stands on the drawn beach is copied (palms, foliage, rocks, tents,
# benches, the MAHARAKA arch, pool water ...), except assets that are logic/culling/test helpers or
# guard-rail pipes. v2 copied 267 props chosen by keyword and left the island bare.
PROP_KEYWORDS = ()
PROP_EXCLUDE = ('PIPE', 'CUL_BOX', 'LV_MODULE_MESH', 'LAND01_LC', 'TST_ADD', 'SKY_', 'NAVI', 'NAVMESH')
# A prop is kept when the original landscape under it is at least this far above the beach waterline
# (metres in Maharaka units); pool water and the pier area sit slightly below/at the waterline.
LAND_MARGIN_BELOW_WATERLINE = 1.5
# The Maharaka materials were tuned for the Maharaka scene light (diffuse 0.8 / ambient 0.25); the Bern
# day scene is about 3x stronger (2.4 / 0.72), which saturates the baked sand albedo and the props to
# white. The catalog colorTint (multiplied into the diffuse) scales every opaque/alpha island asset
# back by about the light ratio. Water surfaces keep tint 1 so they match the Bern sea.
ISLAND_COLOR_TINT = 0.36


def load_catalog():
    assets = {}
    with open(MHP_CATALOG, 'rb') as handle:
        for line in handle.read().decode('utf-8').split('\n')[1:]:
            if line.strip():
                assets[shlex.split(line)[0]] = line
    return assets


_ROW = re.compile(r'^(\d+) "([^"]*)" "([^"]*)" "(\w+)" "([^"]+)" (.*)$')


def load_mhp_rows():
    """[(asset, source, [11 number strings])] of every visible Maharaka placement."""
    rows = []
    with open(MHP_PLACEMENTS, 'rb') as handle:
        for line in handle.read().decode('utf-8', 'replace').split('\n')[1:]:
            match = _ROW.match(line)
            if not match:
                continue
            numbers = match.group(6).split()
            if len(numbers) != 11 or numbers[-1] != '1':
                continue
            rows.append((match.group(5), match.group(2), numbers))
    return rows


def load_tiles():
    """[(assetId, modelPath, catalogLine, (px, py, pz))] for the 16 Maharaka landscape tiles."""
    assets = load_catalog()
    tiles = []
    for asset, _source, numbers in load_mhp_rows():
        if 'LAND01_LC' not in asset or asset not in assets:
            continue
        tiles.append((asset, shlex.split(assets[asset])[2], assets[asset],
                      (float(numbers[0]), float(numbers[1]), float(numbers[2]))))
    if len(tiles) != 16:
        raise RuntimeError('expected 16 Maharaka landscape tiles, found %d' % len(tiles))
    tiles.sort(key=lambda t: (t[3][2], t[3][0]))
    return tiles


def blob_center(tiles):
    """Centroid (x, z) in Maharaka coordinates of terrain above the shared waterline."""
    sx = sz = count = 0.0
    for _asset, model, _line, position in tiles:
        vertices, _tri = read_static_mesh(os.path.join(RESOURCES, model))
        above = vertices[:, 1] * MODEL_SCALE > WATERLINE_ORIGINAL_Y
        sx += float(np.sum(position[0] + vertices[above, 0] * MODEL_SCALE))
        sz += float(np.sum(position[2] + vertices[above, 2] * MODEL_SCALE))
        count += float(np.count_nonzero(above))
    return sx / count, sz / count


def offset(tiles):
    """Pivot in Maharaka coordinates that maps onto ISLAND_CENTER."""
    return blob_center(tiles)


def bern_xz(x, z, pivot):
    return ISLAND_CENTER[0] + (x - pivot[0]) * SCALE, ISLAND_CENTER[1] + (z - pivot[1]) * SCALE


def bern_position(tile_position, pivot):
    bx, bz = bern_xz(tile_position[0], tile_position[2], pivot)
    return (bx, PLACEMENT_Y, bz)


class GroundSampler:
    """Original landscape height (Maharaka metres) at an x/z, rasterised at 1 m."""
    CELL = 1.0

    def __init__(self, tiles):
        from bern_terrain import new_height_grid, rasterise_max_height, world_triangles
        self.x0, self.z0 = -6.0, -1050.0
        self.gw, self.gh = 190, 160
        self.grid = new_height_grid(self.gw, self.gh)
        for _asset, model, _line, position in tiles:
            triangles = world_triangles(os.path.join(RESOURCES, model), position)
            rasterise_max_height(self.grid, triangles, self.x0, self.z0, self.CELL, self.gw, self.gh)

    def height(self, x, z):
        gx = int((x - self.x0) / self.CELL)
        gz = int((z - self.z0) / self.CELL)
        if 0 <= gx < self.gw and 0 <= gz < self.gh and np.isfinite(self.grid[gz, gx]):
            return float(self.grid[gz, gx])
        return None

    def sea_height(self, x, z, default=WATERLINE_ORIGINAL_Y):
        value = self.height(x, z)
        return default if value is None else value


def placement_id(source):
    """Deterministic editor-domain ID (< 2^63) from the stable source string."""
    digest = hashlib.sha256(source.encode('utf-8')).digest()
    return int.from_bytes(digest[:8], 'little') & 0x7FFFFFFFFFFFFFFF or 1


def format_row(source, asset, position, quaternion, scale):
    parts = [str(placement_id(source)), '"%s"' % source, '"%s"' % PLACEMENT_LEVEL, '"editor"', '"%s"' % asset]
    parts += [repr(round(v, 5)) for v in position]
    parts += list(quaternion)
    parts += [repr(round(v, 6)) for v in scale]
    parts.append('1')
    return ' '.join(parts)


def tile_rows(tiles, pivot):
    rows = []
    for asset, _model, _line, position in tiles:
        x, y, z = bern_position(position, pivot)
        source = '%s:landscape:mhp:%s' % (PLACEMENT_LEVEL, asset)
        rows.append((source, asset, format_row(source, asset, (x, y, z), ('0', '0', '0', '1'), TERRAIN_SCALE), None))
    return rows


def prop_selected(asset):
    upper = asset.upper()
    if PROP_KEYWORDS and not any(k in upper for k in PROP_KEYWORDS):
        return False
    return not any(k in upper for k in PROP_EXCLUDE)


WATER_PLANE_MAX_SCALE = 20.0
WATER_SURFACE_LIFT = 0.15


def is_water_surface(asset):
    upper = asset.upper()
    return 'WATER0' in upper


def tinted_catalog_line(asset, line):
    """Catalog row with the island colorTint applied (opaque / alpha assets, not water)."""
    if is_water_surface(asset):
        return line
    parts = line.split(' ')
    for i in range(len(parts) - 1):
        if parts[i] in ('Opaque', 'Alpha') and parts[i + 1] in ('Back', 'Front', 'None'):
            break
    else:
        return line          # Sky / Additive / Water render modes stay untinted
    numbers = parts[i + 2:]
    # uvScaleXY uvSpeedXY opacity emissive specular specularPower tintXYZW opacityPower
    if len(numbers) != 13:
        raise RuntimeError('unexpected catalog render profile for ' + asset)
    tint = repr(ISLAND_COLOR_TINT)
    numbers[8:11] = [tint, tint, tint]
    return ' '.join(parts[:i + 2] + numbers)


def ground_following_y(y, ground_y):
    return SEA_SURFACE_Y + (ground_y - WATERLINE_ORIGINAL_Y) * VERTICAL_SCALE + (y - ground_y) * SCALE


def prop_rows(pivot, ground, assets):
    """Selected Maharaka props with uniform scale and terrain-following height.

    Returns [(newSource, asset, row, originalSource)]."""
    rows = []
    for asset, source, numbers in load_mhp_rows():
        if asset not in assets or not prop_selected(asset):
            continue
        x, y, z = float(numbers[0]), float(numbers[1]), float(numbers[2])
        land = ground.height(x, z)
        if land is None or land < WATERLINE_ORIGINAL_Y - LAND_MARGIN_BELOW_WATERLINE:
            continue
        g = ground.sea_height(x, z)
        bx, bz = bern_xz(x, z, pivot)
        by = ground_following_y(y, g)
        if is_water_surface(asset):
            if max(abs(float(v)) for v in numbers[7:10]) >= WATER_PLANE_MAX_SCALE:
                continue        # the 100x arena-wide water plane would tile the whole sea around the island
            # Pool water is authored a few centimetres above the beach waterline, i.e. at the level of the
            # surrounding sea. Keep it above the Bern sea planes (10.7 - 10.9 m) so it is not swallowed by
            # them; raised deck pools keep their ground-following height.
            by = max(by, SEA_SURFACE_Y + WATER_SURFACE_LIFT)
        scale = tuple(float(v) * SCALE for v in numbers[7:10])
        new_source = '%s:prop:%s' % (PLACEMENT_LEVEL, source)
        rows.append((new_source, asset, format_row(new_source, asset, (bx, by, bz), numbers[3:7], scale), source))
    return rows
