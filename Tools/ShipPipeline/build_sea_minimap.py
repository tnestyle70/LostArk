"""Top-down minimap picture of the Bern sea window (Data/UI/Minimap/MinimapAreas.json, area "Bern Sea").

The minimap is one image whose UV window scrolls with the player. The retail sea minimap belongs to the
retail voyage zone (other coordinates), so this project draws its own: the land outline is the same land
the ship navigation keeps out of (Bern navigation ground, the un-rendered Bern landscape above the waterline
and the ISL_00072 island mesh above the waterline), so the map coast and where a ship can actually go agree.

Orientation is the Bern minimap contract (see CMinimapView): retail cm x = client X * 100, y = -client Z * 100,
image right = +y (client -Z), image up = +x (client +X).

The island position is read from the Gameplay.world.json dock trigger, not typed in here.

Usage: python build_sea_minimap.py [--stats] [--write]   (default: dry run, writes only the preview)
"""
import argparse
import json
import math
import os
import shutil
import sys

import cv2
import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import bern_isl72 as isl72  # noqa: E402
import bern_terrain as terrain  # noqa: E402
import build_sea_nav as nav  # noqa: E402

ROOT = isl72.ROOT
AREA = isl72.BERN_AREA
IMAGE_REL = 'UI/Minimap/Maps/BernSea.png'
IMAGE_PATH = os.path.join(ROOT, 'Client', 'Bin', 'Resources', *IMAGE_REL.split('/'))
MIRROR_ROOT = r'C:\Users\USER\OneDrive\바탕 화면\CY_Resources'
PREVIEW = r'C:\Users\USER\.claude\jobs\46aea322\tmp\sea_minimap\BernSea_preview.png'
WORLD_DOC = os.path.join(ROOT, 'Data', 'Worlds', AREA, 'Gameplay.world.json')
DOCK_ID = 'island.dock.to.maharaka'

# Sea navigation window (Tools/ShipPipeline/build_sea_nav.py RECT) plus a 20 m margin on every side.
NAV_RECT = nav.RECT
MARGIN_M = 20.0
IMG_RECT = (NAV_RECT[0] - MARGIN_M, NAV_RECT[1] + MARGIN_M, NAV_RECT[2] - MARGIN_M, NAV_RECT[3] + MARGIN_M)
CELL = nav.CELL
PX_PER_M = 2.5   # same scale convention as the retail town images used by the other areas

WATER_DEEP = np.array([22.0, 58.0, 78.0])
WATER_SHALLOW = np.array([52.0, 112.0, 124.0])
LAND = np.array([176.0, 160.0, 118.0])
LAND_EDGE = np.array([70.0, 58.0, 40.0])
MARKER = (255, 214, 64)
SHALLOW_FALLOFF_M = 14.0
FOCUS_REACH_M = 30.0
RING_RADIUS_M = 16.0


def land_mask():
    """1 where the ship must not go and the ground is land, on the 0.5 m cell grid of IMG_RECT."""
    x0, x1, z0, z1 = IMG_RECT
    width = int(round((x1 - x0) / CELL))
    height = int(round((z1 - z0) / CELL))

    ground = np.zeros((height, width), dtype=np.uint8)
    for region in ('Bern', 'Bern3', 'Bern2'):
        for (px, pz) in nav.Source(region).cells():
            gx = int(math.floor((px - x0) / CELL))
            gz = int(math.floor((pz - z0) / CELL))
            if 0 <= gx < width and 0 <= gz < height:
                ground[gz, gx] = 1

    waterline = isl72.SEA_SURFACE_Y - nav.TERRAIN_WATERLINE_MARGIN_M
    height_bern = terrain.new_height_grid(width, height)
    tiles = nav.read_bern_landscape_tiles(IMG_RECT)
    for path, position in tiles:
        terrain.rasterise_max_height(height_bern, terrain.world_triangles(path, position), x0, z0, CELL, width, height)
    bern_land = (height_bern > waterline).astype(np.uint8)

    height_island = terrain.new_height_grid(width, height)
    terrain.rasterise_max_height(height_island, isl72.world_triangles(), x0, z0, CELL, width, height)
    island_land = (height_island > waterline).astype(np.uint8)

    land = (ground | bern_land | island_land).astype(np.uint8)
    land = cv2.morphologyEx(land, cv2.MORPH_CLOSE, np.ones((3, 3), np.uint8))
    count, labels, stats, _ = cv2.connectedComponentsWithStats(land, connectivity=8)
    for index in range(1, count):
        if stats[index, cv2.CC_STAT_AREA] < 24:     # 6 m^2 specks
            land[labels == index] = 0
    return land, {'ground': int(ground.sum()), 'bern_terrain': int(bern_land.sum()),
                  'island': int(island_land.sum()), 'tiles': len(tiles), 'width': width, 'height': height}


def orient(grid):
    """[gz, gx] grid (x east, z north) -> image rows along +x descending, columns along y = -z ascending."""
    return np.ascontiguousarray(grid.T[::-1, ::-1])


def world_to_pixel(x, z):
    col = (-z - (-IMG_RECT[3])) * PX_PER_M
    row = (IMG_RECT[1] - x) * PX_PER_M
    return col, row


def read_dock():
    with open(WORLD_DOC, 'r', encoding='utf-8') as handle:
        doc = json.load(handle)
    for placement in doc['placements']:
        if placement.get('placementId') == DOCK_ID:
            return placement['position'][0], placement['position'][2]
    raise SystemExit('dock trigger %s not found in %s' % (DOCK_ID, WORLD_DOC))


def render(land, dock):
    out_w = int(round((IMG_RECT[3] - IMG_RECT[2]) * PX_PER_M))
    out_h = int(round((IMG_RECT[1] - IMG_RECT[0]) * PX_PER_M))
    free = (land == 0).astype(np.uint8)
    distance = cv2.distanceTransform(free, cv2.DIST_L2, 5) * CELL
    shallow = np.exp(-distance / SHALLOW_FALLOFF_M).astype(np.float32)

    land_img = cv2.resize(orient(land).astype(np.float32), (out_w, out_h), interpolation=cv2.INTER_LINEAR)
    shallow_img = cv2.resize(orient(shallow), (out_w, out_h), interpolation=cv2.INTER_LINEAR)

    water = WATER_DEEP[None, None, :] * (1.0 - shallow_img[:, :, None]) + WATER_SHALLOW[None, None, :] * shallow_img[:, :, None]
    rgb = water * (1.0 - land_img[:, :, None]) + LAND[None, None, :] * land_img[:, :, None]
    ring = np.clip(1.0 - np.abs(land_img - 0.5) * 2.0, 0.0, 1.0) * 0.6
    rgb = rgb * (1.0 - ring[:, :, None]) + LAND_EDGE[None, None, :] * ring[:, :, None]
    # A little noise breaks the 8-bit banding of the far-water gradient into invisible steps.
    rgb = rgb + np.random.default_rng(7).uniform(-0.6, 0.6, rgb.shape)
    image = np.clip(rgb, 0, 255).astype(np.uint8)

    # The ring is centred on the island's sand (land within reach of the dock point) so it holds both the
    # sand and the dock; without land nearby it falls back to the dock point itself.
    focus = dock
    x0, x1, z0, z1 = IMG_RECT
    gz_idx, gx_idx = np.nonzero(land)
    if gx_idx.size:
        world_x = x0 + (gx_idx + 0.5) * CELL
        world_z = z0 + (gz_idx + 0.5) * CELL
        near = (world_x - dock[0]) ** 2 + (world_z - dock[1]) ** 2 <= FOCUS_REACH_M ** 2
        if near.any():
            focus = (float(world_x[near].mean()), float(world_z[near].mean()))
    col, row = world_to_pixel(focus[0], focus[1])
    shift = 4
    centre = (int(round(col * (1 << shift))), int(round(row * (1 << shift))))
    radius = int(round(RING_RADIUS_M * PX_PER_M * (1 << shift)))
    cv2.circle(image, centre, radius, LAND_EDGE.tolist(), 5, cv2.LINE_AA, shift)
    cv2.circle(image, centre, radius, MARKER, 3, cv2.LINE_AA, shift)
    return image, (col, row), focus


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--write', action='store_true', help='install the PNG into Resources and CY_Resources')
    parser.add_argument('--stats', action='store_true')
    args = parser.parse_args()

    land, info = land_mask()
    dock = read_dock()
    image, pixel, focus = render(land, dock)
    os.makedirs(os.path.dirname(PREVIEW), exist_ok=True)
    Image.fromarray(image).save(PREVIEW)
    height, width = image.shape[:2]

    x_cm = (int(round(IMG_RECT[0] * 100)), int(round(IMG_RECT[1] * 100)))
    y_cm = (int(round(-IMG_RECT[3] * 100)), int(round(-IMG_RECT[2] * 100)))
    print('cells', info)
    print('image', width, 'x', height, 'px  scale', PX_PER_M, 'px/m')
    print('worldMinCm', [x_cm[0], y_cm[0]], 'worldMaxCm', [x_cm[1], y_cm[1]])
    print('dock trigger (client x,z)', dock, ' ring focus (client x,z)', tuple(round(v, 1) for v in focus))
    print('ring pixel (col,row)', tuple(round(v, 1) for v in pixel),
          'uv', (round(pixel[0] / width, 4), round(pixel[1] / height, 4)))
    print('land fraction', round(float(land.mean()), 4))

    if args.stats:
        # Where the water column starts per client z band: first x (east) at which >50% of the band is sea.
        x0, x1, z0, z1 = IMG_RECT
        for zc in range(-660, -140, 40):
            gz = int((zc - z0) / CELL)
            band = land[max(gz - 20, 0):gz + 20, :]
            sea = (band.mean(axis=0) < 0.5)
            first = np.argmax(sea) if sea.any() else -1
            print('z', zc, 'first sea x =', None if first < 0 else round(x0 + first * CELL, 1),
                  ' land columns', int((~sea).sum()))

    if args.write:
        os.makedirs(os.path.dirname(IMAGE_PATH), exist_ok=True)
        Image.fromarray(image).save(IMAGE_PATH, optimize=True)
        mirror = os.path.join(MIRROR_ROOT, *IMAGE_REL.split('/'))
        os.makedirs(os.path.dirname(mirror), exist_ok=True)
        shutil.copyfile(IMAGE_PATH, mirror)
        print('wrote', IMAGE_PATH, os.path.getsize(IMAGE_PATH), 'bytes; mirror', mirror)
    return 0


if __name__ == '__main__':
    sys.exit(main())
