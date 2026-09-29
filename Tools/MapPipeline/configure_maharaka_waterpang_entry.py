"""Configure saved G jumps, room intro trigger and mesh-baked arena support.

Does not move user markers, NPCs, permanent map placements or camera keys.
CAS install with a before/candidate backup; publishers own runtime outputs.
The focused navigation region preserves the existing flat base outside the
exact 18 stage tiles, centre disc and two top barrel meshes. It spans the whole
base grid (0.25 m cells): the Server answers a query from the region that holds
its first point alone, so a region smaller than the island trapped every player
who waded into it. The arena detail is baked in a 22 m window inside it.
"""
import argparse
import copy
import json
import math
from pathlib import Path
import sys
import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT/'Tools/WorldPipeline'))
sys.path.insert(0, str(ROOT/'Tools/ModelAssetConverter'))
sys.path.insert(0, str(ROOT/'Tools/EffectPipeline'))
from test_maharaka_npc_population import rows, mesh_triangles, transform
from source_character_registration import commit_staged_files

AREA = 'LV_OCN_EVENTIS_MHP'
STAGE = 'world.sequence.instance.maharaka.waterpang.source.intro15.stage'
OUT = ROOT/'out/MaharakaWaterpangEntry20260928'


def encode(value):
    return (json.dumps(value, ensure_ascii=False, indent=2)+'\n').encode('utf-8')


def prepare():
    world_path = ROOT/f'Data/Worlds/{AREA}/Gameplay.world.json'
    sequence_path = ROOT/f'Data/Maps/Authoring/{AREA}/{AREA}.worldsequences.json'
    placement_path = ROOT/f'Data/Maps/Authoring/{AREA}/{AREA}.mapplacements'
    catalog_path = ROOT/f'Data/Maps/Imported/{AREA}/{AREA}.mapassets'
    paths = [world_path, sequence_path, placement_path, catalog_path]
    expected = {p:p.read_bytes() for p in paths}
    world = json.loads(expected[world_path].decode('utf-8-sig'))
    sequence = json.loads(expected[sequence_path].decode('utf-8-sig'))
    before_world = copy.deepcopy(world)
    by_id = {p['placementId']:p for p in world['placements']}
    assert len(by_id) == len(world['placements'])
    for n in range(1,4):
        start, end = by_id[f'jump{n}'], by_id[f'jump{n}_1']
        assert start['kind'] == end['kind'] == 'triggerBox'
        event = {'type':'movePlayer', 'targetPosition':end['position'],
                 'durationSeconds':1.2, 'arcHeight':2.5}
        assert not start['events'] or start['events'] == [event], 'Existing jump changed; review before replacing'
        start.update(enabled=True, triggerOnce=False, requiresInteract=True,
                     interactAction='climb' if n == 3 else 'tightrope', events=[event])
    start_trigger = {'placementId':'waterpang.arena.start', 'kind':'triggerBox',
                     'position':[75.05,22.65,-984.32], 'yawDegrees':0,
                     'enabled':True, 'halfExtents':[5.5,0.7,5.5],
                     'triggerOnce':False, 'requiresInteract':False,
                     'events':[{'type':'playSequence','sequenceInstanceId':STAGE}]}
    if start_trigger['placementId'] in by_id:
        assert by_id[start_trigger['placementId']] == start_trigger
    else:
        world['placements'].append(start_trigger)
    if world != before_world:
        world['revision'] += 1
    old_sequence = copy.deepcopy(sequence)
    for suffix in ('mokomoko','cannon'):
        inst, = [i for i in sequence['instances'] if i['instanceId'] == STAGE.rsplit('.',1)[0]+'.'+suffix]
        assert inst['motionEnd'] in ('STOP','HOLD')
        inst['motionEnd'] = 'HOLD'
    if sequence != old_sequence:
        sequence['revision'] += 1
    placements = {p[0]:p for p in rows(placement_path)}
    catalog = {p[0]:p for p in rows(catalog_path)}
    stage, = [i for i in sequence['instances'] if i['instanceId'] == STAGE]
    ids = [b['targetId'] for b in stage['bindings']]
    # Exact scene export identities, verified against installed mesh bounds.
    ids += ['11877804709865735844','11550822667285194357','10829680542090774286']
    assert len(ids)==21 and len(set(ids))==21
    # Arena window measured against the meshes; ARENA_* mirrors Shared MaharakaWaterpangContract.h.
    width=88; size=.25; ox=64.; oz=-995.
    xx,zz=np.meshgrid(ox+(np.arange(width)+.5)*size,oz+(np.arange(width)+.5)*size)
    heights=np.full(xx.shape,20.48); count=0
    for pid in ids:
        placement=placements[pid]
        model=ROOT/'Client/Bin/Resources'/catalog[placement[4]][2]
        expected[model]=model.read_bytes()
        for tri in mesh_triangles(model):
            a,b,c=[np.array(transform(v,placement)) for v in tri]
            normal=np.cross(b-a,c-a)
            if abs(normal[1])<1e-8 or abs(normal[1])/np.linalg.norm(normal)<math.cos(math.radians(50)):
                continue
            den=(b[2]-c[2])*(a[0]-c[0])+(c[0]-b[0])*(a[2]-c[2])
            u=((b[2]-c[2])*(xx-c[0])+(c[0]-b[0])*(zz-c[2]))/den
            v=((c[2]-a[2])*(xx-c[0])+(a[0]-c[0])*(zz-c[2]))/den
            y=u*a[1]+v*b[1]+(1-u-v)*c[1]
            mask=(u>=-1e-6)&(v>=-1e-6)&(u+v<=1.000001)&(y>heights)&(y<22.8)
            heights[mask]=y[mask]; count+=1
    assert np.max(heights)>22.39
    for n in range(1,4):
        x,y,z=by_id[f'jump{n}_1']['position']
        support=float(heights[int((z-oz)/size),int((x-ox)/size)])
        assert 22.3<support<22.5, (n,support)
        assert all(abs(by_id[f'jump{n}_1']['position'][j]-start_trigger['position'][j])<=start_trigger['halfExtents'][j] for j in (0,2))
    region='WaterpangEntry'
    # The region equals the base grid footprint (Data/Navigation/<Area>.navgrid.json), flat outside the window.
    region_width=640; region_ox=0.; region_oz=-1072.
    column=int(round((ox-region_ox)/size)); row=int(round((oz-region_oz)/size))
    assert region_width==int(round(160./size)) and column+width<=region_width and row+width<=region_width
    full=np.full((region_width,region_width),20.48)
    full[row:row+width,column:column+width]=heights
    lines=[f'LOSTARK_NAVGRID_SOURCE 1 "{AREA}.{region}" {region_width} {region_width} {size} {region_ox} {region_oz} {region_width*region_width}']
    lines += [f'{x} {z} 1 {full[z,x]:.8f}' for z in range(region_width) for x in range(region_width)]
    nav_path=ROOT/f'Data/Navigation/{AREA}.{region}.navsource'
    manifest=ROOT/f'Data/Navigation/{AREA}.navregions'
    manifest_bytes=f'LOSTARK_NAVGRID_REGIONS 1 "{AREA}" 1\nREGION "{region}" 0.6\n'.encode()
    assert not manifest.exists() or manifest.read_bytes()==manifest_bytes, 'Existing regions must be merged explicitly'
    staged={world_path:(expected[world_path],encode(world)),sequence_path:(expected[sequence_path],encode(sequence)),
            nav_path:(nav_path.read_bytes() if nav_path.exists() else None, ('\n'.join(lines)+'\n').encode()),
            manifest:(manifest.read_bytes() if manifest.exists() else None,manifest_bytes)}
    # Every unrequested actor/marker stays byte-value identical.
    after={p['placementId']:p for p in world['placements']}
    for row in before_world['placements']:
        if row['placementId'] not in ('jump1','jump2','jump3'):
            assert after[row['placementId']]==row
    report={'placementCount':len(world['placements']),'jumpPairs':3,'holdActors':2,
            'navigationMeshPlacementIds':ids,'navigationRaisedCells':int(np.sum(heights>20.49)),
            'worldRevision':world['revision'],'sequenceRevision':sequence['revision']}
    return staged,expected,report


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--apply',action='store_true')
    args=parser.parse_args()
    staged,expected,report=prepare()
    for path,(before,after) in staged.items():
        candidate=OUT/'candidate'/path.relative_to(ROOT)
        candidate.parent.mkdir(parents=True,exist_ok=True); candidate.write_bytes(after)
        backup=OUT/'before'/path.relative_to(ROOT)
        if before is not None and not backup.exists():
            backup.parent.mkdir(parents=True,exist_ok=True); backup.write_bytes(before)
    OUT.mkdir(parents=True,exist_ok=True)
    (OUT/'report.json').write_bytes(encode(report))
    if args.apply: commit_staged_files(staged,expected=expected)
    print(json.dumps(dict(applied=args.apply,**report),ensure_ascii=False))
