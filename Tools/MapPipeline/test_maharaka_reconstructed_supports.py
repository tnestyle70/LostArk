"""Original Mokomoko stand replacement and retained central reconstruction checks."""
import copy
import json
import math
from pathlib import Path
import re
import struct
import unittest
from restore_maharaka_original_stand import ASSET, SOURCE, top_at, mesh_triangles

ROOT = Path(__file__).resolve().parents[2]
AREA = 'LV_OCN_EVENTIS_MHP'
AUTHORING = ROOT / 'Data/Maps/Authoring' / AREA
RESOURCES = ROOT / 'Client/Bin/Resources'
SOURCES = {
    'MAP_MHP_RECON_SUPPORT_COLUMN': 'MAP_45CFCC4B82EE_BG_PAP_NIATOWN_COLUMN01_SM',
    'MAP_MHP_RECON_SUPPORT_TIMBER': 'MAP_7E328EAB6CCD_BG_PAP_NIATOWN_FENCE01B_SM',
    'MAP_MHP_RECON_CENTRAL_PILLAR': 'MAP_12CD7B3924ED_BG_BER_RANIAT_PILLAR04_SM_PSY_OVR_6109C9D9F804',
}


def rows(path):
    lines = path.read_text(encoding='utf-8-sig').splitlines()
    return [[a or b for a, b in re.findall(r'"([^"]*)"|(\S+)', line)]
            for line in lines[1:] if line.strip()]


def mesh_vertices(path):
    data = path.read_bytes()
    assert data[:4] == b'WINT' and data[16:20] == b'WMOD'
    count = struct.unpack_from('<I', data, 20)[0]
    sections = [struct.unpack_from('<IIQQ40s', data, 48+i*64) for i in range(count)]
    section, = [s for s in sections if s[0] == 1]
    offset = 32 + section[2]
    header = struct.unpack_from('<4sIIIIIIIB3s', data, offset)
    assert header[0] == b'WMSH' and header[2] == 0
    start = offset + 36 + header[1]*48
    assert start+header[4]*header[5] <= 16+section[2]+section[3]
    return [struct.unpack_from('<3f', data, start+i*header[4]) for i in range(header[5])]


def world_vertices(row, vertices):
    position = list(map(float, row[5:8]))
    qx, qy, qz, qw = map(float, row[8:12])
    scale = list(map(float, row[12:15]))
    for v in vertices:
        a = [v[i]*scale[i]*.01 for i in range(3)]
        t = [2*(qy*a[2]-qz*a[1]), 2*(qz*a[0]-qx*a[2]), 2*(qx*a[1]-qy*a[0])]
        cross = [qy*t[2]-qz*t[1], qz*t[0]-qx*t[2], qx*t[1]-qy*t[0]]
        yield [position[i]+a[i]+qw*t[i]+cross[i] for i in range(3)]


class ReconstructedSupportsTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.catalog = {r[0]: r for r in rows(ROOT/'Data/Maps/Imported'/AREA/(AREA+'.mapassets'))}
        cls.placements = rows(AUTHORING/(AREA+'.mapplacements'))
        cls.supports = [r for r in cls.placements if r[1].startswith('reconstruction.maharaka.')]
        cls.materials = json.loads((AUTHORING/(AREA+'.mapmaterials.json')).read_text(encoding='utf-8-sig'))
        cls.world = json.loads((ROOT/'Data/Worlds'/AREA/'Gameplay.world.json').read_text(encoding='utf-8-sig'))

    def test_stable_additive_placements(self):
        self.assertEqual(len(self.supports), 1)
        self.assertEqual(len({r[0] for r in self.placements}), len(self.placements))
        self.assertEqual(len({r[1] for r in self.supports}), 1)
        self.assertEqual(self.supports[0][1], 'reconstruction.maharaka.central.pillar')
        original, = [r for r in self.placements if r[1] == SOURCE]
        self.assertEqual(original[4], ASSET)
        self.assertEqual(original[2], AREA+'_SL01')
        self.assertEqual(list(map(float, original[5:8])),
                         [75.17852050781251, 19.904390869140624, -1007.261640625])
        self.assertEqual(list(map(float, original[12:15])), [1, 1, 1])
        for row in self.supports:
            self.assertEqual(row[2:4], [AREA+'_SL01', 'overlay'])
            self.assertIn(row[4], SOURCES)
            self.assertTrue(0 < int(row[0]) < 2**63)
            self.assertTrue(all(math.isfinite(float(v)) for v in row[5:15]))
            self.assertAlmostEqual(sum(float(v)**2 for v in row[8:12]), 1, places=7)
            self.assertTrue(all(float(v) > 0 for v in row[12:15]))
            self.assertEqual(row[15], '1')

    def test_original_models_and_surface_inputs_reused_without_wrong_rnm(self):
        for target, source in SOURCES.items():
            self.assertEqual(self.catalog[target][2], self.catalog[source][2])
            self.assertTrue((RESOURCES/self.catalog[target][2]).is_file())
            self.assertIn('not original placement', self.catalog[target][10])
            expected = [copy.deepcopy(m) for m in self.materials['materials'] if m['assetId'] == source]
            actual = [m for m in self.materials['materials'] if m['assetId'] == target]
            self.assertTrue(expected)
            for m in expected:
                m['assetId'] = target
                m.pop('bakedLighting', None)
            self.assertEqual(actual, expected)
        self.assertFalse(any(p['assetId'] in SOURCES for p in self.materials['placementLighting']))

    def test_support_bounds_and_head_contact(self):
        vertices = {a: mesh_vertices(RESOURCES/self.catalog[a][2]) for a in SOURCES}
        heads = {p['placementId']: p for p in self.world['placements']}
        original, = [r for r in self.placements if r[1] == SOURCE]
        head = heads['npc.maharaka.source57009.actor100']
        top = top_at(mesh_triangles(RESOURCES/self.catalog[ASSET][2]), original,
                     head['position'][0], head['position'][2])
        pillar, = [r for r in self.supports if r[1].endswith('central.pillar')]
        points = list(world_vertices(pillar, vertices[pillar[4]]))
        self.assertAlmostEqual(min(v[1] for v in points), 22.5, places=5)
        self.assertAlmostEqual(max(v[1] for v in points), 23.65, places=5)
        # Model-space idle minimum Y measured from the installed skinned models.
        self.assertAlmostEqual(head['position'][1]-.06260592, top, places=6)
        self.assertAlmostEqual(heads['npc.maharaka.source57009.actor188']['position'][1]-.01841349, 23.65)

    def test_published_server_positions_match_authoring(self):
        runtime = {r[0]: r for r in rows(ROOT/'Server/Bin/DataFiles/World/MAHARAKA.worldbootstrap')}
        for p in self.world['placements']:
            actual = list(map(float, runtime[p['placementId']][4:7]))
            self.assertEqual(len(actual), len(p['position']))
            for published, authored in zip(actual, p['position']):
                # Publisher decimal serialization can round the final double digit.
                self.assertAlmostEqual(published, authored, delta=1e-12)

    def test_published_supports_match_authoring(self):
        runtime = {r[0]: r for r in rows(ROOT/'Client/Bin/DataFiles/Map'/(AREA+'.mapplacements'))}
        for row in self.supports+[r for r in self.placements if r[1] == SOURCE]:
            actual = runtime[row[0]]
            self.assertEqual(actual[:5], row[:5])
            for a, b in zip(actual[5:], row[5:]):
                self.assertAlmostEqual(float(a), float(b), places=6)

    def test_original_material_slots_and_resources(self):
        bindings = [r for r in self.materials['materials'] if r['assetId'] == ASSET]
        self.assertEqual([r['materialName'] for r in bindings],
                         ['itr_02453_04_mi', 'itr_02453_01_mi', 'itr_02453_02_mi', 'itr_02453_03_mi'])
        for row, number in zip(bindings, (4, 1, 2, 3)):
            self.assertEqual(row['sourceMaterial'], f'itr_02453.mat.itr_02453_{number:02d}_mi')
            self.assertEqual(row['family'], f'source.character.maharaka-itr02453-{number:02d}.v1')
            self.assertNotIn('bakedLighting', row)
            self.assertEqual(row['renderMode'], 'deferred')
            for texture in row['textures']:
                self.assertTrue((RESOURCES/texture['assetId']).is_file())
                self.assertEqual(Path(texture['assetId']).suffix, '.dds')
        runtime = json.loads((ROOT/'Client/Bin/DataFiles/Map'/(AREA+'.mapmaterials.json')).read_text())
        self.assertEqual([r for r in runtime['materials'] if r['assetId'] == ASSET], bindings)


if __name__ == '__main__':
    unittest.main()
