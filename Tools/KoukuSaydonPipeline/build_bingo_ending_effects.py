"""Project all six active SCENE01B effects with the actual ending body pose.

Writes candidates only. The owner merges authored rows and publishes them after
the source Character clips and World Sequence actors pass their own admission.
"""
from __future__ import annotations
import collections
import copy
import math
from pathlib import Path
import shutil
import sys
import numpy as np
from scipy.spatial.transform import Rotation

import build_bingo_ending_actors as actors
from build_gate2_intro_backdrops import reduced_indices

base, ROOT = actors.base, actors.ROOT
OUT = actors.DEFAULT_OUT / 'ending-effects'
sys.path.insert(0, str(ROOT / 'Tools/EffectPipeline'))
import build_kouku_sequence_effect_groups as groups


def attached_transform(occurrence, rows):
    times = np.array(sorted({round(n*1000/30) for n in range(math.ceil(actors.DURATION*.03))} | {actors.DURATION}))
    poses = [base.world_pose(rows, occurrence['groupExport'], occurrence['actorExport'], t/1000., 32, 45) for t in times]
    positions = np.array([p for p, _ in poses])
    quats = np.array([Rotation.from_matrix(r).as_quat() for _, r in poses])
    eulers = []
    for _, rotation in poses:
        yaw, pitch, roll = Rotation.from_matrix(base.BASIS.T @ rotation @ base.BASIS).as_euler('ZYX', degrees=True)
        eulers.append([-roll, -pitch, yaw])
    eulers = np.rad2deg(np.unwrap(np.deg2rad(eulers), axis=0))
    keep = set(reduced_indices(times, positions, quats))
    pending = list(zip(sorted(keep), sorted(keep)[1:]))
    while pending:
        a, b = pending.pop()
        if b <= a+1: continue
        ratio = (times[a+1:b]-times[a])/(times[b]-times[a])
        errors = np.max(np.abs(eulers[a+1:b]-(eulers[a]*(1-ratio[:, None])+eulers[b]*ratio[:, None])), axis=1)
        index = int(np.argmax(errors))
        if errors[index] > .05:
            k = a+1+index; keep.add(k); pending.extend(((a, k), (k, b)))
    def key(i, value):
        return dict(timeSeconds=float(times[i])/1000., value=list(map(float, value)),
                    arriveTangent=[0, 0, 0], leaveTangent=[0, 0, 0], interpolation='linear')
    position = [key(i, base.BASIS.T @ positions[i]*100) for i in sorted(keep)]
    euler = [key(i, eulers[i]) for i in sorted(keep)]
    prop = rows[occurrence['actorExport']]['p']
    scale = [v*prop.get('drawscale', 1.) for v in groups.vector(prop.get('drawscale3d', {}), (1, 1, 1))]
    node = dict(sourceObjectPath=occurrence['sourceScene'].lower()+'.'+rows[occurrence['actorExport']]['name'],
        frame='WORLD', initialPositionUE3Cm=position[0]['value'], initialEulerDegrees=euler[0]['value'],
        scaleUE3=scale, positionKeys=position, eulerKeys=euler)
    return dict(sourcePositionUE3Cm=groups.vector(prop['location']), rotationDegrees=[0, 0, 0],
                scale=[1, 1, 1], sourceTransformNodes=[node])


def build():
    OUT.mkdir(parents=True, exist_ok=True)
    rows, _ = actors.read_source(actors.DEFAULT_OUT)
    clip = 'kouku.bingo.ending.saydon1'
    model = base.wm.read_wmodel(actors.DEFAULT_OUT/'ending-donors/saydon1.wmodel', include_geometry=False)
    base.bind_bone_model(37, model, 49, clip)
    organization = base.read(ROOT/'out/KoukuAllEffects20260912/organization.json')
    selected = [r for r in organization['sequences']['occurrences'] if r['sourceScene'].endswith('SCENE01B')]
    assert len(selected) == 6
    organization['sequences']['occurrences'] = selected
    base.write(OUT/'organization.json', organization)
    overrides = {r['sourceOccurrenceId']: attached_transform(r, rows) for r in selected if r['actorProperties'].get('basebonename')}
    base.write(OUT/'bone-transforms.json', overrides)
    library = base.read(ROOT/'out/KoukuAllEffectsFinal20260912/installation.json')
    systems = {r['sourceSystem'] for r in selected}
    library['documents'] = [r for r in library['documents'] if r['sourceParticleSystem'] in systems]
    assert len(library['documents']) == 4
    staged = OUT/'library'; (staged/'candidate').mkdir(parents=True, exist_ok=True)
    for row in library['documents']:
        shutil.copy2(ROOT/row['path'], staged/'candidate'/Path(row['path']).name)
    base.write(staged/'installation.json', library)
    groups.project(OUT/'organization.json', staged, ROOT/'out/KoukuAllEffects20260912', OUT/'projected', False, True, overrides)
    report = base.read(OUT/'projected/installation.json')
    assert not report['sourceFailures'], report['sourceFailures']
    installed = []
    for record in report['documents']:
        document = base.read(OUT/'projected/candidate'/Path(record['path']).name)
        by_owner = collections.defaultdict(list)
        for e in document['elements']: by_owner[e['sourceTransformTrack']['sourceOccurrenceId']].append(e)
        chunks = []; elements = []; particles = trails = 0
        for stream in by_owner.values():
            p = sum(round(e['detail']['particle']['maxParticles']*e['detail']['particle']['sourceScale']['count']) for e in stream
                    if e['kind'] == 'particle' or (e.get('sourceRecipe', {}).get('enabled') and e['sourceRecipe'].get('rendererShape') in ('mesh', 'sprite', 'decal')))
            t = sum(e['detail']['trail']['maxPoints'] for e in stream if e['kind'] == 'trail')
            assert p <= 8192 and t <= 2048
            if elements and (particles+p > 8192 or trails+t > 2048):
                chunks.append(elements); elements = []; particles = trails = 0
            elements.extend(stream); particles += p; trails += t
        if elements: chunks.append(elements)
        for index, elements in enumerate(chunks, 1):
            part = copy.deepcopy(document); row = copy.deepcopy(record)
            part['effectAssetId'] += '.'+str(index); part['elements'] = elements
            row.update(effectAssetId=part['effectAssetId'], path='Data/Effects/Authored/'+part['effectAssetId']+'.effect.json', elementCount=len(elements))
            base.write(OUT/'candidate'/Path(row['path']).name, part); installed.append(row)
    report['documents'] = installed
    base.write(OUT/'installation.json', report)
    sequences = base.read(ROOT/'Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json')
    resource_example = next(r for r in sequences['presentationResources'] if r.get('resourceKind') == 'V1_EFFECT')
    occurrence_example = next(o for p in sequences['patterns'] for o in p['presentationOccurrences']
                              if o['resourceId'].startswith('presentation.effect.kouku.sequence.'))
    patch = dict(presentationResources=[], effectCatalogEntries=[], patterns=[])
    for pattern_id in ('KAKULSAYDON_G1_PATTERN_75', 'KAKULSAYDON_G1_PATTERN_9'):
        patch['patterns'].append(dict(patternId=pattern_id, durationMs=max(r['durationMs'] for r in installed), addPresentationOccurrences=[]))
    for index, row in enumerate(installed, 1):
        resource = copy.deepcopy(resource_example)
        resource.update(resourceId='presentation.'+row['effectAssetId'], assetId=row['effectAssetId'],
                        displayName='Bingo ending / original death smoke and exit explosion', defaultAnchorKind='MAP', durationMs=row['durationMs'])
        patch['presentationResources'].append(resource)
        patch['effectCatalogEntries'].append(dict(effectAssetId=row['effectAssetId'], payloadKind='DIRECT_AUTHORED_DOCUMENT', authoringPath=row['path'][5:]))
        origin = row['previewOriginUE3Cm']
        for pattern in patch['patterns']:
            occurrence = copy.deepcopy(occurrence_example)
            occurrence.update(occurrenceId=pattern['patternId']+'.presentation.endingfx.'+str(index),
                resourceId=resource['resourceId'], startMs=0, durationMs=row['durationMs'],
                positionOffset=[origin[0]*.01, origin[2]*.01, -origin[1]*.01], anchorKind='MAP',
                worldId='', worldOccurrenceId='', followBoss=False)
            pattern['addPresentationOccurrences'].append(occurrence)
    base.write(OUT/'composition-field-patch.json', patch)
    return report


if __name__ == '__main__': build()
