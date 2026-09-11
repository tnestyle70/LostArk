"""Project SCENE04A movable lights through the existing V1 typed point lights.

Matinee position is reduced to bounded linear segments; RGB keeps cubic curves. Each segment
uses an ordinary light, the existing transform lerp and alpha distribution;
only three lights can be active. No alternate light runtime is introduced.
"""
from __future__ import annotations
import sys
from pathlib import Path
import numpy as np
import build_gate2_intro_composition as scene

sys.path.insert(0, str(scene.ROOT / 'Tools/LevelPlacementExtractor'))
import build_imported_effect_documents as imported

ASSET = 'effect.kouku.gate2.intro.lights'


def build():
    rows = {r['index']: r for r in scene.read(scene.EVIDENCE / 'source.json')['exports']}
    elements, receipts = [], []
    for group_id, actor_id, component_id in [(498, 972, 969), (431, 973, 970), (432, 974, 971)]:
        group, actor, component = rows[group_id], rows[actor_id], rows[component_id]['p']
        tracks = scene.active_tracks(rows, group_id)
        props = {t['p']['propertyname']: t for t in tracks if 'propertyname' in t['p']}
        color_points = props['lightcolor']['p']['vectortrack']['points']
        brightness_points = props['brightness']['p']['floattrack']['points']
        radius_points = props['radius']['p']['floattrack']['points']

        def sample(t):
            pos, _ = scene.world_pose(rows, group_id, actor_id, t)
            color = scene.curve(color_points, t, [0., 0., 0.])
            intensity = scene.curve(brightness_points, t, 0.)
            radius = scene.curve(radius_points, t, 0.) * .01
            return np.r_[pos, color * intensity, radius]

        boundaries = {0., 27.}
        for track in tracks:
            for value in track['p'].values():
                if isinstance(value, dict):
                    boundaries.update(float(k['inval']) for k in value.get('points', []) if 0 < k.get('inval', -1) < 27)
        segments = []

        def split(a, b):
            left, right = sample(a), sample(b)
            error = max(float(np.max(np.abs(sample(a + (b-a)*q)[:3] - (left[:3]*(1-q)+right[:3]*q)))) for q in [.25, .5, .75])
            if error > .0005 and b-a > .001:
                split(a, (a+b)*.5)
                split((a+b)*.5, b)
            else:
                segments.append((a, b, left, right, error))

        for a, b in zip(sorted(boundaries), sorted(boundaries)[1:]):
            split(a, b)
        maximum_error = 0.
        for index, (a, b, left, right, error) in enumerate(segments):
            maximum_error = max(maximum_error, error)
            if np.max(left[3:6]) <= 0 and np.max(right[3:6]) <= 0:
                continue
            assert left[6] > 0 and abs(left[6]-right[6]) < 1e-8
            detail = imported.default_detail()
            detail['transform']['position'] = left[:3].tolist()
            detail['linearLerp'].update(position=True, endPosition=right[:3].tolist())
            af, bf = np.float32(a), np.float32(b)
            duration = float(np.float32(bf-af))
            detail['timing'].update(startDelaySeconds=float(af), lifeTimeSeconds=duration)
            detail['particle']['initialPositionMin'] = [0., 0., 0.]
            detail['particle']['initialPositionMax'] = [0., 0., 0.]
            detail['light'].update(enabled=True, profileId='light.point.reconstructed.v1', status='reconstructed_profile',
                range=float(left[6]), intensity=1., color=[1.,1.,1.,1.], ambient=[0.,0.,0.,0.],
                falloffExponent=float(component.get('falloffexponent', 2.)))
            distribution = dict(propertyPath='coloroverlife', sourceClass='distributionvectorconstantcurve',
                sourceObjectPath=props['brightness']['name'], componentCount=3, operation=1, randomLockAxes=0,
                lookupTableChunkSize=0, lookupTableNumElements=0, lookupTableTimeScale=0., lookupTableStartTime=0.,
                defaultMinimum=[0.,0.,0.,0.], defaultMaximum=[0.,0.,0.,0.], lookupTable=[], keys=[])
            fit_times = np.array([0., 1./3., 2./3., 1.])
            fit = np.polynomial.polynomial.polyfit(fit_times, np.array([sample(a+(b-a)*q)[3:6] for q in fit_times]), 3)
            for q in [.1, .25, .5, .75, .9]:
                assert np.max(np.abs(np.polynomial.polynomial.polyval(q,fit)-sample(a+(b-a)*q)[3:6])) < 1e-8
            slopes = [fit[1], fit[1]+2*fit[2]+3*fit[3]]
            for key_index, (time, value) in enumerate([(0., left[3:6]), (1., right[3:6])]):
                v = value.tolist()+[0.]
                distribution['keys'].append(dict(time=time, minimum=v, maximum=v, interpolation='cubic',
                    arriveTangentMinimum=slopes[key_index].tolist()+[0.], leaveTangentMinimum=slopes[key_index].tolist()+[0.],
                    arriveTangentMaximum=slopes[key_index].tolist()+[0.], leaveTangentMaximum=slopes[key_index].tolist()+[0.]))
            stable = f'kouku.gate2.intro.light.{group_id}.{index}'
            elements.append(dict(id=stable, displayName=f"SCENE04A {group['p']['groupname']} {a:.3f}s", groupId=ASSET,
                sourceNode='', visible=True, kind='light', resources=[],
                transformInheritance=dict(enabled=False, masterElementId=''),
                material=dict(templateId='effect.standard', sourceMaterialPath='', renderProfile='alpha_two_sided_depth_read', sourceProfile=dict(enabled=False)),
                detail=detail, sourceRecipe=dict(enabled=True, rendererShape='light', emitterDelaySeconds=0.,
                    emitterDurationSeconds=duration, emitterLoopCount=1, bursts=[], modules=[dict(stableId=stable+'.color',
                        className='particlemodulecoloroverlife', objectPath=props['brightness']['name'], literals=[], distributions=[distribution])]),
                sourcePresentation=imported.default_source_presentation()))
        receipts.append(dict(group=group_id, actor=actor_id, component=component_id, segmentCount=len(segments),
            maximumMeasuredPositionErrorM=maximum_error, rgbCubicProjectionError=1e-8, sourceTracks=[t['index'] for t in tracks]))
    doc = dict(schema='lostark.effect-authoring', version=13, effectAssetId=ASSET, displayName='2관문 진입 컷신 이동 광원',
        particleSystem=dict(uniformScaleMultiplier=1, yawOffsetDegrees=0, directionYawDegrees=0, initialSpeedMultiplier=1), modelCues=[], elements=elements)
    scene.write(scene.ROOT / f'Data/Effects/Authored/{ASSET}.effect.json', doc)
    catalog_path = scene.ROOT / 'Data/Effects/EffectCatalog.json'
    catalog = scene.read(catalog_path)
    entry = dict(effectAssetId=ASSET, payloadKind='DIRECT_AUTHORED_DOCUMENT', authoringPath=f'Effects/Authored/{ASSET}.effect.json')
    catalog['effects'] = [e for e in catalog['effects'] if e['effectAssetId'] != ASSET] + [entry]
    scene.write(catalog_path, catalog)
    scene.write(scene.EVIDENCE / 'movable_light_projection.json', dict(effectAssetId=ASSET, elements=len(elements), lights=receipts,
        remainingBoundary='UE3 channel masks and dynamic shadow maps are not represented by the existing V1 point-light carrier; missing explicit falloff uses the existing UE3 inferred exponent 2.'))
    print(f'{ASSET}: {len(elements)} segments for 3 movable lights, position error <=0.0005m; RGB native cubic <=1e-8')


if __name__ == '__main__':
    build()
