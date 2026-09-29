"""Published Maharaka navigation lets a player leave the Waterpang arena on foot.

Re-implements the Server rule (CServerNavigation::Select_Region): the region that
contains the query's first point answers alone, otherwise the base grid answers.
Reads only the published Server navgrid files; run as a test or with --report.
"""
import json
import struct
import sys
import unittest
from collections import deque
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
AREA = 'LV_OCN_EVENTIS_MHP'
NAV = ROOT / 'Server/Bin/DataFiles/Navigation'
MAX_STEP = 0.6  # navpolicy of both grids
# Footprint of the arena the Waterpang hazards act on (Shared MaharakaWaterpangContract.h).
ARENA = (64.0, 86.0, -995.0, -973.0)


class Grid:
    def __init__(self, name):
        data = (NAV / name).read_bytes()
        self.w, self.h, self.size, self.ox, self.oz = struct.unpack_from('<IIfff', data)
        count = self.w * self.h
        self.flags = data[20:20 + count]
        self.heights = struct.unpack_from('<%df' % count, data, 20 + count)

    def contains(self, x, z):
        return (self.ox <= x < self.ox + self.w * self.size and
                self.oz <= z < self.oz + self.h * self.size)

    def cell(self, x, z):
        return int((z - self.oz) / self.size) * self.w + int((x - self.ox) / self.size)

    def walkable(self, index):
        return self.flags[index] != 0

    def height_at(self, x, z):
        return self.heights[self.cell(x, z)]


def load():
    return Grid(f'{AREA}.navgrid'), Grid(f'{AREA}.WaterpangEntry.navgrid')


def select(base, region, x, z):
    return region if region.contains(x, z) else base


def can_walk(base, region, start, goal):
    """True when a walkable 8-neighbour path with steps <= MAX_STEP joins the two points."""
    grid = select(base, region, *start)
    if not grid.contains(*goal):
        return False
    s, g = grid.cell(*start), grid.cell(*goal)
    if not grid.walkable(s) or not grid.walkable(g):
        return False
    seen = {s}
    queue = deque([s])
    while queue:
        cur = queue.popleft()
        if cur == g:
            return True
        cx, cz = cur % grid.w, cur // grid.w
        for dz in (-1, 0, 1):
            for dx in (-1, 0, 1):
                if not dx and not dz:
                    continue
                nx, nz = cx + dx, cz + dz
                if not (0 <= nx < grid.w and 0 <= nz < grid.h):
                    continue
                nxt = nz * grid.w + nx
                if nxt in seen or not grid.walkable(nxt):
                    continue
                if abs(grid.heights[nxt] - grid.heights[cur]) > MAX_STEP:
                    continue
                seen.add(nxt)
                queue.append(nxt)
    return False


def in_arena(x, z):
    return ARENA[0] <= x < ARENA[1] and ARENA[2] <= z < ARENA[3]


# (label, start xz, goal xz). Starts stand on the pool floor (y 20.48) of the arena.
POOL_START = (66.0, -990.0)
CASES = [
    ('pool -> island exit trigger', POOL_START, (62.7, -975.6)),
    ('pool -> spawn (61.18,-975.7)', POOL_START, (61.18, -975.7)),
    ('pool -> spawn (68.36,-968.82)', POOL_START, (68.36, -968.82)),
    ('pool -> spawn (57.55,-982.72)', POOL_START, (57.55, -982.72)),
    ('pool -> east sand (95,-984)', POOL_START, (95.0, -984.0)),
    ('pool -> south pool (75,-1003)', POOL_START, (75.0, -1003.0)),
    ('sand -> pool', (57.55, -982.72), POOL_START),
    ('exit trigger -> pool', (62.7, -975.6), POOL_START),
]


def report():
    base, region = load()
    print(f'base {base.w}x{base.h} @{base.size} origin ({base.ox},{base.oz}) '
          f'walkable={sum(1 for f in base.flags if f)}')
    print(f'region {region.w}x{region.h} @{region.size} origin ({region.ox},{region.oz}) '
          f'walkable={sum(1 for f in region.flags if f)}')
    for label, start, goal in CASES:
        print(f'{label:36s} {"WALK" if can_walk(base, region, start, goal) else "BLOCKED"}')


class WalkableOutsideArena(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.base, cls.region = load()

    def test_client_and_server_grids_match(self):
        for name in (f'{AREA}.navgrid', f'{AREA}.WaterpangEntry.navgrid'):
            self.assertEqual((NAV / name).read_bytes(),
                             (ROOT / 'Client/Bin/DataFiles/Navigation' / name).read_bytes())

    def test_player_can_walk_out_of_the_pool_and_back(self):
        for label, start, goal in CASES:
            with self.subTest(label):
                self.assertTrue(can_walk(self.base, self.region, start, goal))

    def test_region_still_holds_the_arena_deck_detail(self):
        world = json.loads((ROOT / f'Data/Worlds/{AREA}/Gameplay.world.json').read_text(encoding='utf-8-sig'))
        rows = {p['placementId']: p for p in world['placements']}
        for n in range(1, 4):
            x, _, z = rows[f'jump{n}_1']['position']
            self.assertTrue(in_arena(x, z))
            ground = self.region.height_at(x, z)
            self.assertGreater(ground, 22.3)
            self.assertLess(ground, 22.5)

    def test_deck_stays_a_separate_height_layer(self):
        # The raised deck is reached only by the jump triggers, exactly as before.
        deck = (75.05, -984.32)
        self.assertFalse(can_walk(self.base, self.region, POOL_START, deck))

    def test_region_covers_every_walkable_base_cell(self):
        # Standing in the region must never make a base-walkable place unreachable by footprint.
        self.assertLessEqual(self.region.ox, self.base.ox)
        self.assertLessEqual(self.region.oz, self.base.oz)
        self.assertGreaterEqual(self.region.ox + self.region.w * self.region.size,
                                self.base.ox + self.base.w * self.base.size)
        self.assertGreaterEqual(self.region.oz + self.region.h * self.region.size,
                                self.base.oz + self.base.h * self.base.size)


if __name__ == '__main__':
    if '--report' in sys.argv:
        report()
    else:
        unittest.main(verbosity=2)
