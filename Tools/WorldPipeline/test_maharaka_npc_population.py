"""Read-only checks for 28 source scene residents plus the two original NPCs.

Source actors, not guessed missing Deploy NPC definitions or guide identities.
Swimming/sitting actors retain original height; dry-ground projection is wrong.
SurfaceIndex is retained for other terrain checks that import this helper.
"""
import json
import hashlib
import math
from pathlib import Path
import re
import struct
import sys
import unittest

ROOT = Path(__file__).resolve().parents[2]
AREA = 'LV_OCN_EVENTIS_MHP'
PREFIX = 'npc.maharaka.scene'


def read_json(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def rows(path):
    return [[a or b for a, b in re.findall(r'"([^"]*)"|(\S+)', line)]
            for line in path.read_text(encoding='utf-8-sig').splitlines()[1:] if line.strip()]


def mesh_triangles(path):
    data = path.read_bytes()
    assert data[:4] == b'WINT' and data[16:20] == b'WMOD', path
    sections = [struct.unpack_from('<IIQQ40s', data, 48+i*64)
                for i in range(struct.unpack_from('<I', data, 20)[0])]
    section, = [s for s in sections if s[0] == 1]
    offset = 32 + section[2]
    h = struct.unpack_from('<4sIIIIIIIB3s', data, offset)
    assert h[0] == b'WMSH' and h[2] == 0 and h[7] in (2, 4), path
    descriptors = [struct.unpack_from('<IIIIIQ20s', data, offset+36+i*48) for i in range(h[1])]
    start = offset+36+h[1]*48
    index_start = start+h[4]*h[5]
    assert index_start+h[6]*h[7] <= 16+section[2]+section[3]
    for vo, vc, io, ic, _, _, _ in descriptors:
        vertices = [struct.unpack_from('<3f', data, start+vo+i*h[4]) for i in range(vc)]
        indices = struct.unpack_from('<'+('H' if h[7] == 2 else 'I')*ic, data, index_start+io)
        assert ic % 3 == 0 and all(i < vc for i in indices)
        for i in range(0, ic, 3):
            yield tuple(vertices[k] for k in indices[i:i+3])


def transform(v, row):
    qx, qy, qz, qw = map(float, row[8:12])
    a = [v[i]*float(row[12+i])*.01 for i in range(3)]
    t = [2*(qy*a[2]-qz*a[1]), 2*(qz*a[0]-qx*a[2]), 2*(qx*a[1]-qy*a[0])]
    c = [qy*t[2]-qz*t[1], qz*t[0]-qx*t[2], qx*t[1]-qy*t[0]]
    return tuple(float(row[5+i])+a[i]+qw*t[i]+c[i] for i in range(3))


class SurfaceIndex:
    def __init__(self):
        catalog = {r[0]: r for r in rows(ROOT/f'Data/Maps/Imported/{AREA}/{AREA}.mapassets')}
        self.buckets = {'ground': {}, 'water': {}}
        cache = {}
        for r in rows(ROOT/f'Data/Maps/Authoring/{AREA}/{AREA}.mapplacements'):
            if r[15] != '1':
                continue
            asset = catalog[r[4]]
            kind = 'ground' if '_LAND01_LC_' in r[4] else ('water' if asset[11] == 'Water' else None)
            if kind is None:
                continue
            path = ROOT/'Client/Bin/Resources'/asset[2]
            if path not in cache:
                cache[path] = list(mesh_triangles(path))
            for triangle in cache[path]:
                tri = tuple(transform(v, r) for v in triangle)
                # Only the island extent; do not allocate buckets over the whole ocean.
                x0, x1 = max(0, math.floor(min(v[0] for v in tri)/4)), min(40, math.floor(max(v[0] for v in tri)/4))
                z0, z1 = max(-268, math.floor(min(v[2] for v in tri)/4)), min(-228, math.floor(max(v[2] for v in tri)/4))
                for x in range(x0, x1+1):
                    for z in range(z0, z1+1):
                        self.buckets[kind].setdefault((x, z), []).append(tri)

    def height(self, kind, x, z):
        heights = []
        for a, b, c in self.buckets[kind].get((math.floor(x/4), math.floor(z/4)), []):
            den = (b[2]-c[2])*(a[0]-c[0])+(c[0]-b[0])*(a[2]-c[2])
            if abs(den) < 1e-10:
                continue
            u = ((b[2]-c[2])*(x-c[0])+(c[0]-b[0])*(z-c[2]))/den
            v = ((c[2]-a[2])*(x-c[0])+(a[0]-c[0])*(z-c[2]))/den
            if min(u, v, 1-u-v) >= -1e-7:
                heights.append(u*a[1]+v*b[1]+(1-u-v)*c[1])
        return max(heights, default=None)

    def dry_flat(self, x, z, y):
        for dx, dz in ((0, 0), (.6, 0), (-.6, 0), (0, .6), (0, -.6)):
            ground = self.height('ground', x+dx, z+dz)
            water = self.height('water', x+dx, z+dz)
            if ground is None or abs(ground-y) > .12 or (water is not None and ground-water < .25):
                return False
        return True


class MaharakaNpcPopulationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.world = read_json(ROOT/f'Data/Worlds/{AREA}/Gameplay.world.json')
        cls.npcs = [p for p in cls.world['placements'] if p['kind'] == 'npc']
        cls.added = [p for p in cls.npcs if p['placementId'].startswith(PREFIX)]
        cls.catalog = {p['archetypeId']: p for p in read_json(ROOT/'Data/Actors/NpcCatalog.json')['npcs']}
        cls.proof = read_json(ROOT/'Data/Actors/MaharakaResidents.source.json')

    def test_cap_and_originals_preserved(self):
        self.assertEqual(len(self.npcs), 30)
        self.assertEqual(len(self.added), 28)
        self.assertEqual(len({p['placementId'] for p in self.world['placements']}), 34)
        self.assertEqual(len({p['archetypeId'] for p in self.npcs}), 22)
        self.assertFalse(any('reconstructed' in p['placementId'] for p in self.npcs))
        self.assertEqual([p for p in self.world['placements'] if not p['placementId'].startswith(PREFIX)],
                         self.proof['preservedPlacements'])
        originals = [p for p in self.npcs if not p['placementId'].startswith(PREFIX)]
        self.assertEqual({p['placementId']: p['position'] for p in originals}, {
            'npc.maharaka.source57009.actor100': [75.179, 25.19908152, -1007.262],
            'npc.maharaka.source57009.actor188': [75.05, 23.66841349, -984.32]})
        self.assertEqual([p['position'] for p in self.world['placements'] if p['kind'] == 'playerSpawn'],
                         [[61.18, 20.48, -975.7], [68.36, 20.48, -968.82],
                          [63.81, 20.48, -971.06], [57.55, 20.48, -982.72]])

    def test_original_transform_and_no_invented_guide_identity(self):
        actors = {a['placementId']:a for a in self.proof['actors']}
        for p in self.added:
            actor = actors[p['placementId']]
            source = actor['sourceTransform']
            xyz = source['location']
            self.assertEqual(p['position'],[xyz['x']/100,xyz['z']/100,-xyz['y']/100])
            self.assertEqual(p['yawDegrees'],(source['rotation']['degrees']['yaw']+90)%360)
            self.assertIsNone(actor['numericNpcId'])
            self.assertIsNone(actor['guideRole'])
            self.assertEqual(p['archetypeId'],actor['archetypeId'])

    def test_existing_resources_stationary_and_runtime(self):
        runtime = {r[0]: r for r in rows(ROOT/'Server/Bin/DataFiles/World/MAHARAKA.worldbootstrap')}
        for p in self.npcs:
            self.assertTrue(p['enabled'])
            self.assertIsNone(p['behavior'])
            self.assertIsNone(p['idleClip'])
            npc = self.catalog[p['archetypeId']]
            self.assertEqual(npc['runtimeStatus'], 'supported')
            for key in ('modelAssetId', 'animationSetId'):
                if npc[key] is not None:
                    self.assertTrue((ROOT/'Client/Bin/Resources'/npc[key]).is_file(), npc[key])
            self.assertEqual(runtime[p['placementId']][2], p['archetypeId'])
            for emitted, authored in zip(map(float,runtime[p['placementId']][4:7]),p['position']):
                self.assertAlmostEqual(emitted,authored,delta=1e-10)

    def test_cooked_models_and_native_appearance(self):
        sys.path.insert(0, str(ROOT/'Tools/ModelAssetConverter'))
        from verify_dimensionmaster_summon_bind_pose import read_wmodel
        proofs = {m['modelAssetId']:m for m in self.proof['models']}
        materials = read_json(ROOT/'Data/Actors/NpcCatalog.json')['modelMaterialOverrides']
        actors = {a['placementId']:a for a in self.proof['actors']}
        models = {}
        for p in self.added:
            npc = self.catalog[p['archetypeId']]
            body_path = ROOT/'Client/Bin/Resources'/npc['modelAssetId']
            proof = proofs[npc['modelAssetId']]
            self.assertIsNone(npc['animationSetId'])
            self.assertEqual(hashlib.sha256(body_path.read_bytes()).hexdigest(),proof['sha256'])
            if body_path not in models:
                models[body_path] = read_wmodel(body_path,include_geometry=False,animation_names=())
            model = models[body_path]
            self.assertNotIn('RootNode',{b.name for b in model.skeleton_bones})
            self.assertEqual(len(model.animations),1)
            clip = model.animations[0]
            self.assertEqual(clip.name,npc['idleClip'])
            self.assertAlmostEqual(clip.duration_ticks/clip.ticks_per_second,proof['motion']['seconds'],places=4)
            self.assertTrue(proof['motion']['scaleKeysPreserved'])
            self.assertLessEqual(proof['merge']['headBindDeltaM'],.01)
            overrides = [r for r in materials if r['modelAssetId']==npc['modelAssetId']]
            self.assertEqual(len(overrides),2)
            for slot, original in zip(proof['materialSlots'],actors[p['placementId']]['materials']):
                row, = [r for r in overrides if r['materialName']==slot]
                self.assertEqual(row['sourceMaterial'],original['sourceMaterial'])
                self.assertIn(row['family'],('source.character.maharaka-resident-female.v1',
                                            'source.character.maharaka-resident-male.v1'))
                self.assertEqual(len(row['textures']),6)
                for texture in row['textures']:
                    self.assertTrue((ROOT/'Client/Bin/Resources'/texture['assetId']).is_file())
                variation = original['variation']
                if variation:
                    self.assertEqual(row['parameters']['mask_variation_visible'],
                                     [int(variation['maskvariation_'+str(i)]) for i in range(1,5)])
                    for name in ('diffusecolor','diffusecolor_a','diffusecolor_b','diffusecolor_c'):
                        self.assertEqual(row['parameters'][name],[variation[name][c] for c in ('r','g','b','a')])


    def test_skin_bind_and_sampled_loop_geometry(self):
        sys.path.insert(0,str(ROOT/'Tools/ModelAssetConverter'))
        from verify_dimensionmaster_summon_bind_pose import (
            read_wmodel,combined_transforms,matrix_multiply,matrix_inverse,identity_error,sample_animation)
        for row in self.proof['models']:
            model = read_wmodel(ROOT/'Client/Bin/Resources'/row['modelAssetId'])
            combined = combined_transforms(model.skeleton_bones,[b.transform for b in model.skeleton_bones])
            used = {i for v in model.vertices for i,w in zip(v.indices,v.weights) if w>1e-6}
            products = {i:matrix_multiply(model.mesh_bones[i].transform,combined[i]) for i in used}
            inverse_basis = matrix_inverse(products[min(used)])
            worst = max(identity_error(matrix_multiply(value,inverse_basis)) for value in products.values())
            self.assertLess(worst,.002)
            clip = model.animations[0]
            bounds = [sample_animation(model,clip,clip.duration_ticks*t)['bounds'] for t in (0,.25,.5,.75)]
            diagonals = [frame['diagonal'] for frame in bounds]
            self.assertTrue(all(math.isfinite(d) and 10<d<1000 for d in diagonals))
            self.assertGreater(max(diagonals)-min(diagonals),.001)

    def test_source_signed_basis_and_light_input_layout(self):
        sys.path.insert(0,str(ROOT/'Tools/ModelAssetConverter'))
        from cook_wmodel_geometry_contract import parse_skinned_uv_wmodel
        for proof in self.proof['models']:
            parsed = parse_skinned_uv_wmodel((ROOT/'Client/Bin/Resources'/proof['modelAssetId']).read_bytes())
            self.assertEqual(parsed['versionMinor'],5)
            self.assertEqual(parsed['meshHeader'][4],80)
            evidence = proof['basis']
            self.assertEqual(evidence['joinedVertices'],parsed['meshHeader'][5])
            self.assertTrue(evidence['originalVertex76BytesPreserved'])
            self.assertTrue(evidence['nonMeshSectionsPreserved'])
            for desc,row in zip(parsed['submeshes'],evidence['submeshes']):
                signs = [struct.unpack_from('<f',parsed['mesh'],parsed['vertexStart']+desc[0]+i*80+76)[0]
                         for i in range(desc[1])]
                self.assertEqual(hashlib.sha256(b''.join(struct.pack('<f',s) for s in signs)).hexdigest(),
                                 row['handednessSha256'])
                self.assertGreater(sum(s<0 for s in signs),0)
                self.assertGreater(sum(s>0 for s in signs),0)
        paths = [ROOT/f'{part}/Bin/ShaderFiles/Shader_SourceCharacterMaterial.hlsli' for part in ('Engine','Client')]
        texts = [p.read_text(encoding='utf-8') for p in paths]
        self.assertEqual(texts[0],texts[1])
        branch = texts[0].split('if (g_SourceCharacterProgram == 12u',1)[1].split('if (g_SourceCharacterProgram == 6u',1)[0]
        for program in ('1474u','1475u'):
            self.assertIn('g_SourceCharacterProgram == '+program,branch.split('{',1)[0])
        for assignment in ('input.values[2] = float4(uv, 0.f, 0.f);',
                           'input.values[3] = float4(tangentLight, 1.f);',
                           'input.values[5] = float4(tangentView, 1.f);',
                           'input.values[6] = sourcePosition;'):
            self.assertIn(assignment,branch)


if __name__ == '__main__':
    unittest.main()
