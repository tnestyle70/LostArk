"""Reconnect the installed Kouku Effect library to exact Matinee occurrences.

The existing festival document stays the owner of its six source emitters.
Attached hammer/trumpet/hand effects sample the actual baked CModel bone basis
offline and use the existing sourceTransformTrack WORLD curve at runtime.
"""
from __future__ import annotations
import copy
import collections
import json
import math
import shutil
import sys
from pathlib import Path
import numpy as np
from scipy.spatial.transform import Rotation
import build_source_sequences as sequences
from build_gate2_intro_backdrops import reduced_indices

ROOT, OUT, base = sequences.ROOT, sequences.OUT, sequences.base
sys.path.insert(0,str(ROOT/'Tools/EffectPipeline'))
import build_kouku_sequence_effect_groups as groups


def bone_transform(occurrence, rows, duration):
    actor=occurrence['actorExport'];parent=rows[actor]['p']['base']
    matinee,data=occurrence['matineeExport'],occurrence['dataExport']
    if matinee==328:
        asset='Map/KakulSaydon/SourceSequences/kouku.gate2.maze/Kouku/Kouku.wmodel'
        parent_group=397
        rows[parent]['p']['drawscale']=.012053/.01
    else:
        assert matinee==329 and parent==333
        asset='Map/KakulSaydon/Gate2Intro/Saydon/Saydon.wmodel'
        parent_group=467
    base.BONE_MODELS[parent]=(base.wm.read_wmodel(base.RESOURCES/asset),parent_group,{})
    times=np.array(sorted({round(t*1000/30)for t in range(math.ceil(duration*.03)+1)}|{duration}))
    times=times[times<=duration]
    poses=[base.world_pose(rows,occurrence['groupExport'],actor,t/1000.,matinee,data)for t in times]
    positions=np.array([p for p,_ in poses]);quats=np.array([Rotation.from_matrix(r).as_quat()for _,r in poses])
    keep=reduced_indices(times,positions,quats)
    # Euler values belong to UE3 degrees; unwrap each component across 360°.
    eulers=[]
    for _,rotation in poses:
        yaw,pitch,roll=Rotation.from_matrix(base.BASIS.T@rotation@base.BASIS).as_euler('ZYX',degrees=True)
        eulers.append([-roll,-pitch,yaw])
    eulers=np.rad2deg(np.unwrap(np.deg2rad(eulers),axis=0))
    # Include Euler-linear error in the retained sample set, because runtime
    # source nodes interpolate Euler curves rather than world quaternions.
    keep=set(keep)
    pending=[(a,b)for a,b in zip(sorted(keep),sorted(keep)[1:])]
    while pending:
        left,right=pending.pop()
        if right<=left+1:continue
        ratio=(times[left+1:right]-times[left])/(times[right]-times[left])
        delta=np.max(np.abs(eulers[left+1:right]-(eulers[left]*(1-ratio[:,None])+eulers[right]*ratio[:,None])),axis=1)
        at=int(np.argmax(delta))
        if delta[at]>.05:
            index=left+1+at;keep.add(index);pending.extend([(left,index),(index,right)])
    def key(index,value):
        return dict(timeSeconds=float(times[index])/1000.,value=list(map(float,value)),
                    arriveTangent=[0,0,0],leaveTangent=[0,0,0],interpolation='linear')
    position_keys=[key(i,base.BASIS.T@positions[i]*100)for i in sorted(keep)]
    euler_keys=[key(i,eulers[i])for i in sorted(keep)]
    prop=rows[actor]['p'];scale=groups.vector(prop.get('drawscale3d',{}),(1,1,1))
    scale=[x*prop.get('drawscale',1)for x in scale]
    node=dict(sourceObjectPath=occurrence['sourceScene'].lower()+'.'+rows[actor]['name'],frame='WORLD',
              initialPositionUE3Cm=position_keys[0]['value'],initialEulerDegrees=euler_keys[0]['value'],
              scaleUE3=scale,positionKeys=position_keys,eulerKeys=euler_keys)
    return dict(sourcePositionUE3Cm=groups.vector(prop['location']),rotationDegrees=[0,0,0],scale=[1,1,1],sourceTransformNodes=[node])


def main():
    organization=base.read(ROOT/'out/KoukuAllEffects20260912/organization.json')
    wanted={('SCENE03A','efseqact_matinee_0'),('SCENE02A','efseqact_matinee_10'),
            ('SCENE04A','efseqact_matinee_1'),('SCENE04A','efseqact_matinee_2')}
    selected=[];excluded=[];overrides={};cache={}
    for original in organization['sequences']['occurrences']:
        suffix=original['sourceScene'].rsplit('_',1)[-1]
        if (suffix,original['matineeId']) not in wanted:continue
        row=copy.deepcopy(original)
        if row['sourceSystem']=='fx_q_w_01.fx_par.par_q_festiparticle_01':
            excluded.append(dict(sourceOccurrenceId=row['sourceOccurrenceId'],reason='REUSED_EXISTING_FESTIVAL_DOCUMENT'))
            continue
        if suffix not in cache:
            c=next(c for c in sequences.CONFIGS if c['scene']==suffix)
            cache[suffix]=sequences.scene_rows(c)
        if row['actorProperties'].get('basebonename'):
            overrides[row['sourceOccurrenceId']]=bone_transform(row,cache[suffix],11950 if row['matineeExport']==328 else 27000)
        selected.append(row)
        if suffix=='SCENE02A':
            arrival=copy.deepcopy(row)
            intervals=[]
            for interval in arrival['activationIntervals']:
                if interval['startSeconds']>=16.710:
                    intervals.append(interval)
            if intervals:
                arrival['activationIntervals']=intervals
                arrival['matineeId']+='_arrival'
                selected.append(arrival)
    organization['sequences']['occurrences']=selected
    path=OUT/'sequence_effect_organization.json';base.write(path,organization)
    # Read the installed authored documents so finalized native programs win
    # over historical candidate payloads from an earlier material generation.
    library=base.read(ROOT/'out/KoukuAllEffectsFinal20260912/installation.json')
    staged=OUT/'InstalledEffectLibrary';(staged/'candidate').mkdir(parents=True,exist_ok=True)
    for row in library['documents']:
        source=ROOT/row['path'];assert source.is_file(),source
        shutil.copyfile(source,staged/'candidate'/source.name)
    # The final native library intentionally omitted the previously recovered
    # paper/festival family. Reuse its installed material+renderer ABI without
    # copying the festival occurrence clock, spawn layout or particle curves.
    paper_name='effect.kouku.source.fx_q_w_01.fx_par.par_q_coloredpaper_01.effect.json'
    paper=base.read(ROOT/'out/KoukuAllEffectsLightClosure20260912/candidate'/paper_name)
    festival=base.read(ROOT/'Data/Effects/Authored/effect.kouku.gate1.intro.festival.full.restore.effect.json')
    for element in paper['elements']:
        matches=[e for e in festival['elements'] if
                 e['material']['sourceMaterialPath']==element['material']['sourceMaterialPath'] and
                 e['sourceRecipe']['rendererShape']==element['sourceRecipe']['rendererShape']]
        assert matches, ('RECOVERED_PAPER_NATIVE_MATERIAL_MISSING',element['sourceNode'])
        assert all(e['material']==matches[0]['material'] for e in matches)
        element['material']=copy.deepcopy(matches[0]['material'])
    base.write(staged/'candidate'/paper_name,paper)
    library['documents'].append(dict(sourceParticleSystem='fx_q_w_01.fx_par.par_q_coloredpaper_01',
        effectAssetId=paper['effectAssetId'],path='Data/Effects/Authored/'+paper_name))
    base.write(staged/'installation.json',library)
    effect_out=OUT/'RestoredEffects'
    groups.project(path,staged,ROOT/'out/KoukuAllEffects20260912',effect_out,False,True,overrides)
    report=base.read(effect_out/'installation.json')
    assert all(row['complete'] for row in report['documents']),report['sourceFailures']
    # Runtime budgets apply to reserved counts across the complete document,
    # including inactive elements. Split only between source occurrences so
    # emitter-event references never cross a document boundary.
    installed=[];reused_mesh=[]
    for row in report['documents']:
        document=base.read(effect_out/'candidate'/Path(row['path']).name)
        if row['sourceScene'].endswith('SCENE04A') and row['matineeId']=='efseqact_matinee_2':
            # The existing six CardEruption CModel occurrences already own
            # these three source mesh emitters. Keep the other native cardfly
            # emitters without drawing a second copy of the same cards.
            names={'fx_q_w_01.fx_par_02.par_q_cardfly_01.particlespriteemitter_'+str(i) for i in (1,5,6)}
            reuse=[e for e in document['elements'] if e['sourcePresentation']['sourceObjectPath'] in names]
            assert len(reuse)==18 and len({e['sourceTransformTrack']['sourceOccurrenceId']for e in reuse})==6
            reused_mesh += [dict(elementId=e['id'],sourceNode=e['sourceNode'],
                sourceOccurrenceId=e['sourceTransformTrack']['sourceOccurrenceId'],
                carrier='Map/KakulSaydon/Gate2Intro/CardEruption/CardEruption.wmodel')for e in reuse]
            document['elements']=[e for e in document['elements'] if e not in reuse]
        by_occurrence=collections.defaultdict(list)
        for element in document['elements']:
            by_occurrence[element['sourceTransformTrack']['sourceOccurrenceId']].append(element)
        chunks=[];chunk=[];particles=trails=0
        for occurrence_elements in by_occurrence.values():
            p=sum(round(e['detail']['particle']['maxParticles']*e['detail']['particle']['sourceScale']['count'])
                  for e in occurrence_elements if e['kind']=='particle' or
                  (e.get('sourceRecipe',{}).get('enabled') and e['sourceRecipe'].get('rendererShape') in ['mesh','sprite','decal']))
            t=sum(e['detail']['trail']['maxPoints']for e in occurrence_elements if e['kind']=='trail')
            assert p<=8192 and t<=2048,('SOURCE_OCCURRENCE_EXCEEDS_RUNTIME_BUDGET',p,t)
            if chunk and (particles+p>8192 or trails+t>2048):
                chunks.append(chunk);chunk=[];particles=trails=0
            chunk+=occurrence_elements;particles+=p;trails+=t
        if chunk:chunks.append(chunk)
        for ordinal,elements in enumerate(chunks,1):
            projected=copy.deepcopy(document);record=copy.deepcopy(row)
            projected['effectAssetId']+='.'+str(ordinal)
            projected['elements']=elements
            record['effectAssetId']=projected['effectAssetId']
            record['path']='Data/Effects/Authored/'+projected['effectAssetId']+'.effect.json'
            record['elementCount']=len(elements)
            record['sourceOccurrenceIds']=list(dict.fromkeys(e['sourceTransformTrack']['sourceOccurrenceId']for e in elements))
            base.write(effect_out/'split'/Path(record['path']).name,projected)
            installed.append(record)
    report['documents']=installed
    base.write(effect_out/'split_installation.json',report)
    base.write(effect_out/'reused_card_meshes.json',reused_mesh)
    base.write(OUT/'sequence_effect_bone_transforms.json',overrides)
    base.write(OUT/'sequence_effect_scope.json',dict(excluded=excluded,
        reconstructedBoneOccurrenceCount=len(overrides),sampledSpace='WORLD_UE3_CM_FROM_INSTALLED_CMODEL_BONES',
        arrivalPolicy='New activations at or after original 16.710s arrival cut; earlier particle tails remain in full transition preview.'))
    print(json.dumps(dict(documents=len(report['documents']),bones=len(overrides),excluded=excluded)),flush=True)


if __name__=='__main__':main()
