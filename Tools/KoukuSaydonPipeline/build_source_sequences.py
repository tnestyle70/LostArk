"""Project source Matinees into existing Camera/WorldSequence/Composition JSON.

Source packages and previously authored rows are read-only inputs. Resource
output is confined to new per-sequence directories; installation checks a byte
baseline before merging additions into the user's authoring documents.
"""
from __future__ import annotations
import argparse
import copy
import hashlib
import json
import math
import shlex
import sys
import xml.etree.ElementTree as ET
from pathlib import Path
import numpy as np
from scipy.spatial.transform import Rotation
import build_gate2_intro_composition as base
import bake_reflected_static_props as reflected
from build_gate2_intro_backdrops import reduced_indices

ROOT = base.ROOT
sys.path.insert(0, str(ROOT / 'Tools/ModelAssetConverter'))
from apply_world_object_material_source import assign_material_source, assign_map_material_source, inherit_material_source
OUT = ROOT / 'out/KoukuSourceSequenceRestore20260912'
AREA = base.AREA_DIR
CONFIGS = [
    dict(id=4, name='1관문_통합_시퀀스', prefix='kouku.gate1.full', scene='SCENE03A',
         matinee=365, data=857, duration=41488, start=0, gate='GATE1', combat=True),
    dict(id=5, name='2관문_클리어', prefix='kouku.gate2.clear', scene='SCENE02A',
         matinee=63, data=117, duration=35368, start=0, gate='GATE2', combat=False),
    dict(id=6, name='2관문_카드미로', prefix='kouku.gate2.maze', scene='SCENE04A',
         matinee=328, data=393, duration=11950, start=0, gate='GATE2', combat=False),
    dict(id=7, name='3관문_진입', prefix='kouku.gate3.intro', scene='SCENE02A',
         matinee=63, data=117, duration=35368, start=16710, gate='GATE3', combat=True),
]


def scene_rows(config):
    path = OUT / ('LV_LUT_MIDNIGHTC_ED_' + config['scene'] + '.json')
    if not path.exists():
        sys.path.insert(0, str(ROOT / 'Tools/EffectPipeline'))
        import build_kouku_gate1_full_restore as source
        rows, imports = base.extract_scene(source.source_package('Map', 'LV_LUT_MIDNIGHTC_ED_' + config['scene']))
        base.write(path, dict(rows=rows, imports=imports))
    return {int(k): v for k, v in base.read(path)['rows'].items()}


def merge(document, key, identity, additions):
    rows = document.setdefault(key, [])
    existing = {r[identity]: r for r in rows}
    for row in additions:
        old = existing.get(row[identity])
        if old is not None:
            if old != row:
                raise ValueError('Preserve authored row: ' + row[identity])
        else:
            rows.append(row)
            existing[row[identity]] = row


def project_registration(effect_paths):
    """Stage only missing None items, preserving project bytes and user edits."""
    outputs=[]
    includes=sorted('..\\..\\'+p.relative_to(ROOT).as_posix().replace('/','\\') for p in effect_paths)
    for suffix in ('','.filters'):
        path=ROOT/('Client/Default/Client.vcxproj'+suffix)
        before=path.read_bytes();bom=before.startswith(b'\xef\xbb\xbf')
        text=before.decode('utf-8-sig');newline='\r\n' if '\r\n' in text else '\n'
        missing=[value for value in includes if 'Include="'+value+'"' not in text]
        rows=[]
        for value in missing:
            rows.append('    <None Include="'+value+'"'+('>' if suffix else ' />'))
            if suffix:rows+=['      <Filter>96.DataFiles</Filter>','    </None>']
        if rows:
            head,tail=text.rsplit('</Project>',1)
            assert not tail.strip()
            text=head+newline.join(['  <ItemGroup>']+rows+['  </ItemGroup>','</Project>'])+tail
        ET.fromstring(text)
        after=(b'\xef\xbb\xbf' if bom else b'')+text.encode('utf-8')
        outputs.append((path,before,after))
    return outputs


def resource(identity, label, kind, asset, duration, resource_kind=''):
    return dict(resourceId=identity, displayName=label, defaultAnchorKind='WORLD',
                kind=kind, assetId=asset, resourceKind=resource_kind, elementId='',
                durationMs=duration, shape='BOX', colliderKind='GEOMETRY',
                halfExtents=[1, 1, 1], radiusM=3, halfAngleDegrees=45)


def occurrence(pattern_id, ordinal, resource_id, start, end, anchor='WORLD', position=None):
    result = base.presentation_occurrence(resource_id, ordinal, start, end, anchor)
    result['occurrenceId'] = f'{pattern_id}.presentation.{ordinal}'
    if position is not None:
        result['positionOffset'] = position
    return result


def fade(config, rows):
    tracks = [t['p'] for g in rows[config['data']]['p']['interpgroups']
              for t in base.active_tracks(rows, g) if t['cls'] == 'interptrackfade']
    if not tracks:
        return None
    assert len(tracks) == 1
    points = tracks[0]['floattrack']['points']
    begin, end = config['start'] / 1000., config['duration'] / 1000.
    # V2 has linear/smoothstep keys but no UE3 tangents. Reduce sampled Hermite
    # to a bounded linear opacity curve, retaining the exact source boundaries.
    times = sorted({begin, end} | {p['inval'] for p in points if begin < p['inval'] < end}
                   | {t / 1000. for t in range(config['start'], config['duration'], 20)})
    values = [float(base.curve(points, t, 0.)) for t in times]
    keep = {0, len(times)-1}
    while True:
        worst = (.003, None)
        ordered = sorted(keep)
        for left, right in zip(ordered, ordered[1:]):
            for i in range(left+1, right):
                u = (times[i]-times[left])/(times[right]-times[left])
                error = abs(values[i]-(values[left]*(1-u)+values[right]*u))
                if error > worst[0]:
                    worst = (error, i)
        if worst[1] is None:
            break
        keep.add(worst[1])
    assert len(keep) <= 64, (config['prefix'], 'fade key limit', len(keep))
    identity = config['prefix'] + '.fade.black'
    leaf = base.read(ROOT / 'Data/Effects/V2/Authored/kouku.gate2.intro.fade.black.effectv2.json')
    leaf.update(effectId=identity, displayName=config['name'] + ' / 원본 암전')
    leaf['params']['lifetime'] = end-begin
    post = leaf['params']['screenPost']
    post['intensitySmoothstep'] = False
    post['intensityKeys'] = [dict(timeSeconds=round(times[i]-begin, 7), intensity=values[i]) for i in sorted(keep)]
    return leaf


def animation_tracks(rows, group, model):
    clips = {base.clip_name(a.name): a for a in model.animations}
    tracks = {r['p'].get('slotname', 'a'): r['p'] for r in base.active_tracks(rows, group)
              if r['cls'] == 'interptrackanimcontrol' and r['p'].get('animseqs')}
    assert set(tracks) <= {'a', 'b'} and 'a' in tracks
    for track in tracks.values():
        for key in track['animseqs']:
            assert key['animseqname'] in clips, (group, key['animseqname'])
    return tracks, clips


def bake(config, rows, group, source, label):
    model = base.wm.read_wmodel(base.RESOURCES / source)
    tracks, clips = animation_tracks(rows, group, model)
    duration = config['duration']-config['start']
    destination = base.RESOURCES / f'Map/KakulSaydon/SourceSequences/{config["prefix"]}/{label}/{label}.wmodel'
    clip = config['prefix'] + '.' + label.lower()
    if destination.exists():
        checked = base.wm.read_wmodel(destination)
        assert len(checked.animations) == 1 and checked.animations[0].name == clip
        return destination.relative_to(base.RESOURCES).as_posix(), clip
    samples = []
    for frame in range(math.ceil(duration * .03)+1):
        seconds = min(config['duration']/1000., config['start']/1000.+frame/30.)
        a, t = base.anim_at(tracks['a'], clips, seconds)
        pose = base.pose_sample(model, a, t)
        if 'b' in tracks:
            b, t = base.anim_at(tracks['b'], clips, seconds)
            other = base.pose_sample(model, b, t)
            alpha = float(base.curve(tracks['a'].get('floattrack', {}).get('points', []), seconds, 1.))
            pose = [(bp*(1-alpha)+ap*alpha, base.slerp(bq, aq, alpha), bs*(1-alpha)+asc*alpha)
                    for (ap, aq, asc), (bp, bq, bs) in zip(pose, other)]
        samples.append(pose)
    base.write_clip(base.RESOURCES/source, destination, model, samples, clip, label)
    return destination.relative_to(base.RESOURCES).as_posix(), clip


def actor_world(config, rows, group, source, label, pre_scale, source_scale, material_source=''):
    asset, clip = bake(config, rows, group, source, label)
    actor = base.group_actor(rows, group, config['matinee'])[0]
    identity = config['prefix'] + '.' + label.lower()
    obj = dict(objectId='world.object.'+identity, displayName=config['name']+' / '+label,
               modelAssetId=asset, animated=True, modelPreScale=pre_scale, scale=[1, 1, 1],
               anchorKind='WORLD', diffuseTextureAssetId='', sequenceInstanceId='',
               defaultMotionInstanceId='world.sequence.instance.'+identity)
    if material_source:
        obj['materialSourceModelAssetId'] = material_source
    obj = inherit_material_source(obj, source, ROOT)
    if label == 'Book':
        original = base.read(AREA/(base.AREA+'.worldsequences.json'))
        obj['mapMaterialBindings'] = copy.deepcopy(next(r for r in original['objectResources']
            if r['objectId']=='world.object.kouku.popup.book')['mapMaterialBindings'])
        base.BONE_MODELS[actor] = (base.wm.read_wmodel(base.RESOURCES/asset), group, {})
    duration = config['duration']-config['start']
    keys = []
    # All original property boundaries, with a 200ms upper sampling interval.
    times = base.source_times(rows, group, config['start'], config['duration'], 200)
    for ms in times:
        pos, rotation = base.world_pose(rows, group, actor, ms/1000., config['matinee'], config['data'])
        q = Rotation.from_matrix(rotation).as_quat()
        keys.append(dict(timeMs=ms-config['start'], positionOffset=pos.tolist(),
                         rotationQuaternion=q.tolist(), scaleMultiplier=[source_scale]*3, visible=True))
    template = dict(sequenceId='sequence.'+identity, displayName=obj['displayName'], category='World',
                    durationMs=duration, interpolation='LINEAR', tracks=[dict(slotId='actor', keys=keys)],
                    animationTracks=[dict(slotId='actor', clipName=clip, startMs=0, playbackRate=1,
                                          loop=False, holdLastFrame=True)])
    instance = dict(instanceId=obj['defaultMotionInstanceId'], templateId=template['sequenceId'], enabled=True,
                    startDelayMs=0, playbackSpeed=1, anchorKind='WORLD', position=[0, 0, 0],
                    motionEnd='STOP', nextMotionId='', bindings=[dict(slotId='actor', targetKind='OBJECT_RESOURCE', targetId=obj['objectId'])])
    world = dict(worldId='world.'+identity, displayName=obj['displayName'], sequenceInstanceId=instance['instanceId'],
                 positionOffset=[0, 0, 0], anchorKind='NONE', anchorPosition=[0, 0, 0], companionEffectResourceId='')
    return obj, template, instance, world


def static_worlds(config, rows):
    """Retain source-owned map props and their parent/bone coordinate systems."""
    groups = rows[config['data']]['p']['interpgroups']
    owners = {a: g for g in groups for a in base.group_actor(rows,g,config['matinee'])}
    owned = set(owners)
    while True:
        attached = {i for i,r in rows.items() if r['p'].get('base') in owned}
        if attached <= owned:
            break
        owned |= attached
    imports = base.read(OUT/('LV_LUT_MIDNIGHTC_ED_'+config['scene']+'.json'))['imports']
    catalog = [shlex.split(line) for line in (ROOT/'Client/Bin/DataFiles/Map'/
               (base.AREA+'.mapassets')).read_text(encoding='utf-8-sig').splitlines()[1:]]
    materials = base.read(AREA/(base.AREA+'.mapmaterials.json'))['materials']
    sampled, resources, failures, reflection_cache = [], {}, [], {}
    for actor in sorted(owned):
        p = rows[actor]['p']
        if 'staticmeshcomponent' not in p:
            continue
        try:
            component = rows[p['staticmeshcomponent']]['p']
            source_mesh = imports[str(component['staticmesh'])]
            matches = [r for r in catalog if r[1].casefold()==source_mesh.rsplit('.',1)[-1].casefold()
                       and '_RNM_' not in r[0]]
            for slot,material in enumerate(component.get('materials',[])):
                if material:
                    source_material=imports[str(material)]
                    matches=[candidate for candidate in matches if any(m['assetId']==candidate[0] and
                        m['materialName'].startswith(f'SLOT_{slot:03}_') and m.get('sourceMaterial')==source_material
                        for m in materials)]
            canonical=[r for r in matches if '_OVR_' not in r[0]]
            if len(canonical)==1:
                matches=canonical
            assert len(matches)==1, 'SOURCE_MAP_MESH_IDENTITY_UNRESOLVED'
            asset, model = matches[0][0], matches[0][2]
            bindings = [r for r in materials if r['assetId']==asset]
            assert bindings and all(r['family']=='bg-source-opaque-masked' for r in bindings), 'SOURCE_MAP_MATERIAL_FAMILY_UNRESOLVED'
            assert (base.RESOURCES/model).exists(), 'SOURCE_MAP_MODEL_MISSING'
            for slot,material in enumerate(component.get('materials',[])):
                if material:
                    source_material=imports[str(material)]
                    assert any(r['materialName'].startswith(f'SLOT_{slot:03}_') and
                               r.get('sourceMaterial')==source_material for r in bindings), 'SOURCE_MAP_MATERIAL_OVERRIDE_UNRESOLVED'
            scale=base.vec(p.get('drawscale3d'),(1,1,1))[[0,2,1]]*p.get('drawscale',1.)
            object_id='world.object.'+config['prefix']+'.set.'+asset.lower()
            if np.any(scale<0):
                signs=tuple(-1 if v<0 else 1 for v in scale)
                key=(model,signs)
                if key not in reflection_cache:
                    reflection_cache[key]=reflected.bake_reflected_asset(base.RESOURCES,base.RESOURCES,model,signs)
                variant=reflection_cache[key]
                model=variant['modelAssetId'];object_id+='.reflect.'+variant['tag']
                scale=np.abs(scale)
            # A WorldSequence target is identified by object resource ID. Two
            # independently moving source actors cannot share that target ID.
            object_id+='.actor.'+str(actor)
            resources.setdefault(object_id,dict(objectId=object_id,displayName=source_mesh.rsplit('.',1)[-1],
                modelAssetId=model,anchorKind='WORLD',diffuseTextureAssetId='',modelPreScale=.01,animated=False,
                scale=[1,1,1],sequenceInstanceId='',defaultMotionInstanceId='',mapMaterialBindings=[
                    dict(materialName=r['materialName'],sourceAssetId=asset,sourceMaterialName=r['materialName'])for r in bindings]))
            group=owners.get(actor)
            times=base.source_times(rows,group,config['start'],config['duration'],250)
            keys=[]
            for ms in times:
                pos,rot=base.world_pose(rows,group,actor,ms/1000.,config['matinee'],config['data'])
                keys.append(dict(timeMs=ms-config['start'],positionOffset=pos.tolist(),
                    rotationQuaternion=Rotation.from_matrix(rot).as_quat().tolist(),scaleMultiplier=scale.tolist(),visible=True))
            keep=reduced_indices(np.array(times),np.array([k['positionOffset']for k in keys]),
                                 np.array([k['rotationQuaternion']for k in keys]))
            keys=[keys[i]for i in keep]
            sampled.append((actor,object_id,keys))
        except (AssertionError,KeyError,ValueError) as error:
            failures.append(dict(actor=actor,reason=str(error)))
    known_missing={672,691,692,811,812,843,844,845,847,848,849,850,851,852} if config['id']==4 else set()
    unexpected=[f for f in failures if f['actor'] not in known_missing or f['reason']!='SOURCE_MAP_MESH_IDENTITY_UNRESOLVED']
    if unexpected:raise ValueError('Source background projection failed: '+json.dumps(unexpected))
    outputs=[]
    for index in range(0,len(sampled),32):
        chunk=sampled[index:index+32];identity=config['prefix']+'.set.'+str(index//32+1)
        template=dict(sequenceId='sequence.'+identity,displayName=config['name']+' / 원본 배경 '+str(index//32+1),
            category='World',durationMs=config['duration']-config['start'],interpolation='LINEAR',
            tracks=[dict(slotId='actor.'+str(a),keys=k)for a,_,k in chunk],animationTracks=[])
        instance=dict(instanceId='world.sequence.instance.'+identity,templateId=template['sequenceId'],enabled=True,
            startDelayMs=0,playbackSpeed=1,anchorKind='WORLD',position=[0,0,0],motionEnd='STOP',nextMotionId='',
            bindings=[dict(slotId='actor.'+str(a),targetKind='OBJECT_RESOURCE',targetId=o)for a,o,_ in chunk])
        world=dict(worldId='world.'+identity,displayName=template['displayName'],sequenceInstanceId=instance['instanceId'],
            positionOffset=[0,0,0],anchorKind='NONE',anchorPosition=[0,0,0],companionEffectResourceId='')
        outputs.append((template,instance,world))
    return list(resources.values()),outputs,failures,len(sampled)


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--install', action='store_true')
    args = parser.parse_args()
    paths = dict(composition=ROOT/'Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json',
                 cameras=AREA/(base.AREA+'.camerashots.json'), worlds=AREA/(base.AREA+'.worldsequences.json'),
                 independent=ROOT/'Data/Effects/V2/Independent.json',catalog=ROOT/'Data/Effects/EffectCatalog.json')
    baseline = {k: p.read_bytes() for k, p in paths.items()}
    docs = {k: json.loads(v.decode('utf-8-sig')) for k, v in baseline.items()}
    # Both hand objects use the extracted LEFT static mesh/MIC 08. The source
    # model identity is explicit; hand names do not select material variants.
    map_sources = {
        'world.object.kouku.bingo': 'MAP_5F1286085DD9_LV_LUT_MIDNIGHTC_FLOOR03_SM',
        'world.object.kouku.g3.outer_fire.d': 'MAP_CFEDE8067300_BG_RAD_KOUKUSATON_DECO24D_SM_KHB',
        'world.object.kouku.g3.outer_fire.e': 'MAP_B71A2EC9D778_BG_RAD_KOUKUSATON_DECO24E_SM_KHB',
        'world.object.kouku.g3.outer_fire.f': 'MAP_7AC8BB3D2FEE_BG_RAD_KOUKUSATON_DECO24F_SM_KHB',
    }
    for index, obj in enumerate(docs['worlds']['objectResources']):
        if obj['objectId'] in ('world.object.kouku.saydon_showtime_gun_left',
                              'world.object.kouku.saydon_showtime_gun_right'):
            docs['worlds']['objectResources'][index] = assign_material_source(obj,
                'Character/KoukuSaton/WP_MN_RPCT_07/wp_mn_rpct_07l_sk.wmodel', ROOT)
        if obj['objectId'] in map_sources:
            docs['worlds']['objectResources'][index] = assign_map_material_source(obj,
                map_sources[obj['objectId']], base.AREA, ROOT)
    def bind_world(world):
        previous=next((row for row in docs['composition']['worlds']
                       if row['sequenceInstanceId']==world['sequenceInstanceId']),None)
        if previous:
            world['worldId']=previous['worldId']
        else:
            world['worldId']='kakulsaydon.g1.world.'+str(docs['composition']['nextWorldOrdinal'])
            docs['composition']['nextWorldOrdinal']+=1
        merge(docs['composition'],'worlds','worldId',[world])
    output_files = {}
    effect_manifest=OUT/'RestoredEffects/split_installation.json'
    effect_rows=base.read(effect_manifest)['documents'] if effect_manifest.exists() else []
    if args.install and not effect_rows:
        raise ValueError('Generate source Sequence effects before installing the complete timeline')
    for row in effect_rows:
        output_files[ROOT/row['path']]=base.read(OUT/'RestoredEffects/split'/Path(row['path']).name)
        merge(docs['catalog'],'effects','effectAssetId',[dict(effectAssetId=row['effectAssetId'],
            payloadKind='DIRECT_AUTHORED_DOCUMENT',authoringPath=row['path'].removeprefix('Data/'))])
    def add_effects(config,pattern,presentations):
        matinee={4:'efseqact_matinee_0',5:'efseqact_matinee_10',6:'efseqact_matinee_1',7:'efseqact_matinee_10_arrival',3:'efseqact_matinee_2'}[config['id']]
        for row in effect_rows:
            if row['sourceScene'].endswith(config['scene']) and row['matineeId']==matinee:
                start=round(row['previewTimeOriginSeconds']*1000)-config['start']
                end=min(config['duration']-config['start'],start+row['durationMs'])
                assert 0<=start<end
                r=resource('presentation.'+row['effectAssetId'],config['name']+' / 원본 이펙트 '+row['effectAssetId'].rsplit('.',1)[-1],
                    'EFFECT',row['effectAssetId'],row['durationMs'],'V1_EFFECT')
                presentations.append(r)
                ue=row['previewOriginUE3Cm'];position=[ue[0]*.01,ue[2]*.01,-ue[1]*.01]
                existing=[o for o in pattern['presentationOccurrences'] if o['resourceId']==r['resourceId']]
                if existing:
                    assert len(existing)==1 and existing[0]['startMs']==start and existing[0]['durationMs']==end-start, 'Preserve edited Sequence effect timing'
                    continue
                pattern['presentationOccurrences'].append(occurrence(pattern['patternId'],
                    len(pattern['presentationOccurrences'])+1,r['resourceId'],start,end,'MAP',position))
    summaries = []
    for c in CONFIGS:
        rows = scene_rows(c)
        shots = base.make_cameras(rows, c['matinee'], c['data'], c['duration'], c['prefix'], c['name'], c['start'])
        merge(docs['cameras'], 'shots', 'shotId', [s for _, _, s in shots])
        pid = 'KAKULSAYDON_G1_PATTERN_'+str(c['id'])
        p = dict(patternId=pid, actorProfileId='MN_RPCT_05' if c['gate'] != 'GATE2' else 'MN_RPCZ_00',
                 gateId=c['gate'], targetBossPlacementId='boss.kakulsaydon.'+dict(GATE1='g1.saydon',GATE2='g2.kouku',GATE3='g3.saydon')[c['gate']],
                 displayName=c['name'], authoringStatus='DRAFT', category='MECHANIC', enterCombatOnFinish=c['combat'],
                 nextStageOrdinal=2, nextAnimationOrdinal=1, nextLogicOccurrenceOrdinal=1,
                 nextSummonOccurrenceOrdinal=1, nextWorldOccurrenceOrdinal=1, nextSceneProfileOccurrenceOrdinal=1,
                 stages=[dict(stageId='STAGE_1', actionId=pid+'.stage.1', stageKind='ACTIVE',
                              durationMs=c['duration']-c['start'], animationOccurrences=[])],
                 logicOccurrences=[], summonOccurrences=[], worldOccurrences=[], sceneProfileOccurrences=[],
                 resetBossToSpawn=False, presentationOccurrences=[])
        if c['id'] in (4,7):
            prior=next((row for row in docs['composition']['sceneProfiles']
                        if row['renderingProfileId']=='scene.kakulsaydon.g1.base.v1'),None)
            profile_id=prior['sceneProfileId'] if prior else 'kakulsaydon.g1.sceneprofile.'+str(docs['composition']['nextSceneProfileOrdinal'])
            if not prior:docs['composition']['nextSceneProfileOrdinal']+=1
            profile=dict(sceneProfileId=profile_id,displayName='입장_기본환경',
                         renderingProfileId='scene.kakulsaydon.g1.base.v1')
            merge(docs['composition'],'sceneProfiles','sceneProfileId',[profile])
            p['sceneProfileOccurrences']=[dict(occurrenceId=pid+'.sceneprofile.1',
                sceneProfileId=profile['sceneProfileId'],startMs=0,durationMs=c['duration']-c['start'],blendMs=0)]
            p['nextSceneProfileOccurrenceOrdinal']=2
        presentations = []
        for start, end, shot in shots:
            r = resource('presentation.'+shot['shotId'], shot['displayName'], 'CAMERA', shot['shotId'], end-start)
            presentations.append(r)
            p['presentationOccurrences'].append(occurrence(pid, len(p['presentationOccurrences'])+1, r['resourceId'], start, end))
        leaf = fade(c, rows)
        if leaf:
            output_files[ROOT/'Data/Effects/V2/Authored'/(leaf['effectId']+'.effectv2.json')] = leaf
            if leaf['effectId'] not in docs['independent']['effects']:
                docs['independent']['effects'].append(leaf['effectId'])
            r = resource('presentation.'+leaf['effectId'], leaf['displayName'], 'EFFECT', leaf['effectId'], c['duration']-c['start'], 'LEAF')
            presentations.append(r)
            p['presentationOccurrences'].append(occurrence(pid,len(p['presentationOccurrences'])+1,r['resourceId'],0,c['duration']-c['start'],'MAP'))
        add_effects(c,p,presentations)
        if c['id']==4:
            festival=base.read(ROOT/'out/KoukuFireworks20260911/preview_origin.json')['0']
            effect_id='effect.kouku.gate1.intro.festival.full.restore'
            assert any(r['effectAssetId']==effect_id for r in docs['catalog']['effects'])
            start=round(festival['previewTimeOriginSeconds']*1000)
            r=resource('presentation.'+c['prefix']+'.festival',c['name']+' / 기존 원본 축포','EFFECT',effect_id,c['duration']-start,'V1_EFFECT')
            presentations.append(r);ue=festival['previewOriginUE3Cm']
            p['presentationOccurrences'].append(occurrence(pid,len(p['presentationOccurrences'])+1,r['resourceId'],start,c['duration'],
                'MAP',[ue['x']*.01,ue['z']*.01,-ue['y']*.01]))
        actor_specs = []
        if c['id'] == 4:
            actor_specs = [(1042,'Map/LV_LUT_MIDNIGHTC_ED/AnimatedProps/DEPLOY_CINE_KOUKU_BOOK/DEPLOY_CINE_KOUKU_BOOK.wmodel','Book',.01,2.,''),
                           (1059,'Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel','SaydonBook',.017,1.,'Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel'),
                           (1062,'Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel','SaydonStage',.017,1.,'Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel'),
                           (998,'Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel','SaydonFinale',.017,1.,'Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel')]
        if c['id'] == 6:
            actor_specs = [(397,'Character/KoukuSaton/MN_RPCZ_00/MN_RPCZ_00.wmodel','Kouku',.012053,1.,'Character/KoukuSaton/MN_RPCZ_00/MN_RPCZ_00.wmodel')]
        if c['id'] == 5:
            actor_specs = [(152,'Character/KoukuSaton/MN_RPCT_06/MN_RPCT_06.wmodel','LargeSaydon',.0692,1.,'Character/KoukuSaton/MN_RPCT_06/MN_RPCT_06.wmodel'),
                           (135,'Character/KoukuSaton/MN_RPCZ_00/MN_RPCZ_00.wmodel','Kouku',.012053,1.,'Character/KoukuSaton/MN_RPCZ_00/MN_RPCZ_00.wmodel'),
                           (153,'Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel','SaydonArrival',.017,1.,'Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel')]
        if c['id'] == 7:
            actor_specs = [(153,'Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel','SaydonArrival',.017,1.,'Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel')]
        for spec in actor_specs:
            obj, template, instance, world = actor_world(c, rows, *spec)
            merge(docs['worlds'],'objectResources','objectId',[obj])
            merge(docs['worlds'],'templates','sequenceId',[template])
            merge(docs['worlds'],'instances','instanceId',[instance])
            bind_world(world)
            p['worldOccurrences'].append(dict(occurrenceId=pid+'.world.'+str(len(p['worldOccurrences'])+1),
                worldId=world['worldId'],startMs=0,durationMs=c['duration']-c['start'],playbackSpeed=1))
        props,sets,failures,static_count=static_worlds(c,rows)
        merge(docs['worlds'],'objectResources','objectId',props)
        for template,instance,world in sets:
            merge(docs['worlds'],'templates','sequenceId',[template])
            merge(docs['worlds'],'instances','instanceId',[instance])
            bind_world(world)
            p['worldOccurrences'].append(dict(occurrenceId=pid+'.world.'+str(len(p['worldOccurrences'])+1),
                worldId=world['worldId'],startMs=0,durationMs=c['duration']-c['start'],playbackSpeed=1))
        p['nextWorldOccurrenceOrdinal'] = len(p['worldOccurrences'])+1
        p['nextPresentationOccurrenceOrdinal'] = len(p['presentationOccurrences'])+1
        merge(docs['composition'],'presentationResources','resourceId',presentations)
        merge(docs['composition'],'patterns','patternId',[p])
        summaries.append(dict(patternId=pid,sourceScene=c['scene'],sourceMatinee=c['matinee'],
                              sourceStartMs=c['start'],durationMs=c['duration']-c['start'],
                              cameraShots=len(shots),worldActors=len(actor_specs),staticActors=static_count,
                              sourceStaticFailures=failures))
    docs['composition']['nextPatternOrdinal'] = max(8,docs['composition']['nextPatternOrdinal'])
    entry = next(p for p in docs['composition']['patterns'] if p['patternId']=='KAKULSAYDON_G1_PATTERN_3')
    entry['enterCombatOnFinish'] = True
    intro_effects=[]
    add_effects(dict(id=3,scene='SCENE04A',name=entry['displayName'],start=0,duration=27000),entry,intro_effects)
    merge(docs['composition'],'presentationResources','resourceId',intro_effects)
    entry['nextPresentationOccurrenceOrdinal']=len(entry['presentationOccurrences'])+1
    for key in ['composition','cameras','worlds']:
        if 'revision' in docs[key]:
            docs[key]['revision'] += 1
    for key, document in docs.items():
        output_files[paths[key]] = document
    assert len(json.dumps(docs['worlds'],ensure_ascii=False,indent=2).encode('utf-8')) < 16*1024*1024, 'World Sequence exceeds runtime byte bound'
    assert len(json.dumps(docs['cameras'],ensure_ascii=False,indent=2).encode('utf-8')) < 2*1024*1024, 'Camera exceeds runtime byte bound'
    assert len(docs['cameras']['shots'])<=128
    for path, document in output_files.items():
        candidate = OUT/'candidate'/path.relative_to(ROOT)
        base.write(candidate,document)
    project_files=project_registration(path for path in output_files if
        '/Effects/Authored/' in path.as_posix() or '/Effects/V2/Authored/' in path.as_posix())
    for path,_,after in project_files:
        candidate=OUT/'candidate'/path.relative_to(ROOT)
        candidate.parent.mkdir(parents=True,exist_ok=True);candidate.write_bytes(after)
    base.write(OUT/'authoring_install_manifest.json',dict(installed=False,
        policy='CAS all baseline paths before copying; install only with compatible rebuilt Client',
        files=[dict(target=path.relative_to(ROOT).as_posix(),
            candidate=(OUT/'candidate'/path.relative_to(ROOT)).relative_to(ROOT).as_posix(),
            baselineSha256=hashlib.sha256(next((baseline[k] for k,p in paths.items() if p==path),path.read_bytes() if path.exists() else b'')).hexdigest() if path.exists() else None,
            candidateSha256=hashlib.sha256((OUT/'candidate'/path.relative_to(ROOT)).read_bytes()).hexdigest())
            for path in output_files]+[dict(target=path.relative_to(ROOT).as_posix(),
                candidate=(OUT/'candidate'/path.relative_to(ROOT)).relative_to(ROOT).as_posix(),
                baselineSha256=hashlib.sha256(before).hexdigest(),candidateSha256=hashlib.sha256(after).hexdigest())
                for path,before,after in project_files]))
    if args.install:
        for key, path in paths.items():
            if path.read_bytes() != baseline[key]:
                raise ValueError('Concurrent authoring changed: '+str(path))
        for path,before,_ in project_files:
            if path.read_bytes()!=before:raise ValueError('Concurrent project changed: '+str(path))
        for path, document in output_files.items():
            if path not in paths.values() and path.exists() and base.read(path)!=document:
                raise ValueError('Existing authored effect differs: '+str(path))
        for path, document in output_files.items():
            base.write(path,document)
        for path,_,after in project_files:path.write_bytes(after)
    base.write(OUT/'sequence_restore.report.json',dict(installed=args.install,patterns=summaries,
               cameraBytes=(OUT/'candidate'/paths['cameras'].relative_to(ROOT)).stat().st_size,
               cameraShots=len(docs['cameras']['shots']),manualVisualValidation='USER_PENDING'))
    print(json.dumps(summaries,ensure_ascii=False),flush=True)


if __name__ == '__main__':
    main()
