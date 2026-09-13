"""Project SCENE03A's bound movable lights onto the authored Gate 1 clock.

Only candidate files are written. The existing V1 source transform/distribution
carriers retain cubic tangents and constant jumps; each source light stays one
element, rather than reserving a light for every linear approximation segment.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
from pathlib import Path

import numpy as np
import build_gate2_intro_lights as previous

scene = previous.scene
imported = previous.imported
ROOT = scene.ROOT
ASSET = "effect.kouku.gate1.popup.movable-lights"
SOURCE_END = 41.488
START_SECONDS = 12.258
DURATION = 58.810 - START_SECONDS
# Existing approved normalpopup landmarks, without importing the mutating old
# out/.../retime_authored_effects.py entry point.
LANDMARKS = [(0., 0.), (8.752, 8.752), (19.741, 18.352),
             (22.244, 20.852), (26.919, 25.981),
             (33.583, 35.409), (41.488, DURATION)]
TRACKS = [1269, 1270, 1271, 1272, 1273, 1274, 1281, 1282]


def retime(seconds):
    for (a, x), (b, y) in zip(LANDMARKS, LANDMARKS[1:]):
        if seconds <= b:
            return x + (seconds-a) * (y-x) / (b-a)
    raise ValueError(f"Source time outside Matinee: {seconds}")


def raw_value(point, field):
    value = point.get(field, 0.)
    return scene.vec(value) if isinstance(value, dict) else np.array([value] * 3)


def interval(points, seconds):
    for a, b in zip(points, points[1:]):
        if a["inval"] <= seconds < b["inval"]:
            return a, b
    return None


def curve_value(points, seconds):
    if seconds <= points[0]["inval"]:
        return raw_value(points[0], "outval")
    pair = interval(points, seconds)
    if pair is None:
        return raw_value(points[-1], "outval")
    a, b = pair
    dt = b["inval"]-a["inval"]
    u = (seconds-a["inval"])/dt
    va, vb = raw_value(a, "outval"), raw_value(b, "outval")
    mode = a.get("interpmode", "cim_linear")
    if mode == "cim_constant":
        return va
    if mode == "cim_linear":
        return va*(1-u)+vb*u
    return ((2*u**3-3*u*u+1)*va + (u**3-2*u*u+u)*dt*raw_value(a, "leavetangent") +
            (-2*u**3+3*u*u)*vb + (u**3-u*u)*dt*raw_value(b, "arrivetangent"))


def derivative(points, seconds, side):
    pair = interval(points, math.nextafter(seconds, side))
    if pair is None:
        return np.zeros(3)
    a, b = pair
    dt = b["inval"]-a["inval"]
    u = (seconds-a["inval"])/dt
    va, vb = raw_value(a, "outval"), raw_value(b, "outval")
    mode = a.get("interpmode", "cim_linear")
    if mode == "cim_constant":
        return np.zeros(3)
    if mode == "cim_linear":
        return (vb-va)/dt
    return ((6*u*u-6*u)*va/dt + (3*u*u-4*u+1)*raw_value(a, "leavetangent") +
            (-6*u*u+6*u)*vb/dt + (3*u*u-2*u)*raw_value(b, "arrivetangent"))


def clock_rate(seconds, side):
    sample = min(SOURCE_END, max(0., math.nextafter(seconds, side)))
    for (a, x), (b, y) in zip(LANDMARKS, LANDMARKS[1:]):
        if sample <= b:
            return (y-x)/(b-a)
    raise ValueError(sample)


def distribution(points, path, prop, normalized=False):
    """Split native Hermite curves at each affine-clock boundary, exactly."""
    assert points and all(a['inval'] < b['inval'] for a, b in zip(points, points[1:]))
    boundaries = sorted({0., SOURCE_END, *[a for a, _ in LANDMARKS],
                         *[p['inval'] for p in points if 0 < p['inval'] < SOURCE_END]})
    result = dict(propertyPath=prop, sourceClass='distributionvectorconstantcurve',
        sourceObjectPath=path, componentCount=3, operation=1, randomLockAxes=0,
        lookupTableChunkSize=0, lookupTableNumElements=0, lookupTableTimeScale=0.,
        lookupTableStartTime=0., defaultMinimum=[0.]*4, defaultMaximum=[0.]*4,
        lookupTable=[], keys=[])
    factor = DURATION if normalized else 1.
    for t in boundaries:
        pair = interval(points, math.nextafter(t, math.inf))
        mode = pair[0].get('interpmode', 'cim_linear') if pair else 'cim_constant'
        interpolation = {'cim_constant': 'constant', 'cim_linear': 'linear'}.get(mode, 'cubic')
        value = curve_value(points, t).tolist()+[0.]
        arrive = (derivative(points, t, -math.inf)/clock_rate(t, -math.inf)*factor).tolist()+[0.]
        leave = (derivative(points, t, math.inf)/clock_rate(t, math.inf)*factor).tolist()+[0.]
        result['keys'].append(dict(time=retime(t)/factor, minimum=value, maximum=value,
            interpolation=interpolation, arriveTangentMinimum=arrive,
            arriveTangentMaximum=arrive, leaveTangentMinimum=leave, leaveTangentMaximum=leave))
    return result


def component_defaults():
    path = ROOT/'out/KoukuGate1FullRestore20260911/source_class_defaults.json'
    records = {x['fullPath']: x for x in scene.read(path)['records']}
    light = records['engine.default__lightcomponent']
    point = records['engine.default__pointlightcomponent']
    return dict(color=light['properties']['LightColor']['value'],
                radius=point['properties']['Radius']['value'],
                falloffexponent=point['properties']['FalloffExponent']['value']), [
        {k: r[k] for k in ['fullPath', 'sourcePackage', 'exportIndex', 'serialSha256']}
        for r in [light, point]]


def transform_keys(points, path, prop):
    return [dict(timeSeconds=k['time'], value=k['minimum'][:3],
                 arriveTangent=k['arriveTangentMinimum'][:3],
                 leaveTangent=k['leaveTangentMinimum'][:3], interpolation=k['interpolation'])
            for k in distribution(points, path, prop)['keys']]


def build(source, destination):
    raw = source.read_bytes()
    source_data = json.loads(raw)
    rows = {int(k): v for k, v in source_data['rows'].items()}
    assert rows[857]['p']['interplength'] == 41.48755645751953
    defaults, provenance = component_defaults()
    elements, receipts, unbound = [], [], []
    for track_id in TRACKS:
        group_id = next(g for g in rows[857]['p']['interpgroups']
                        if track_id in rows[g]['p'].get('interptracks', []))
        actor_ids = scene.group_actor(rows, group_id, matinee=365)
        if not actor_ids:
            unbound.append(dict(track=track_id, group=group_id,
                groupName=rows[group_id]['p']['groupname'], reason='No Matinee linked actor'))
            continue
        assert len(actor_ids) == 1
        actor_id = actor_ids[0]
        actor = rows[actor_id]
        assert not actor['p'].get('base'), 'Parented light requires its actual parent transform chain'
        component_id = actor['p']['lightcomponent']
        component = rows[component_id]
        kind = {'pointlightcomponent': 'point', 'spotlightcomponent': 'spot',
                'directionallightcomponent': 'directional'}[component['cls']]
        props = component['p']
        assert 'lightingchannels' not in props, 'Explicit channel overrides need receiver mapping'
        assert 'bdisablespecular' not in props, 'Explicit specular suppression needs source mapping'
        move = [x for x in scene.active_tracks(rows, group_id) if x['cls'] == 'interptrackmove']
        assert len(move) == 1
        move = move[0]
        assert all(p.get('groupname', 'none') == 'none'
                   for p in move['p'].get('lookuptrack', {}).get('points', []))
        frame = {'imf_relativetoinitial': 'RELATIVE_TO_INITIAL', 'imf_world': 'WORLD'}[
            move['p'].get('moveframe', 'imf_world')]
        rotation = actor['p'].get('rotation', {}).get('degrees', {})
        node = dict(sourceObjectPath=actor['name'], frame=frame,
            initialPositionUE3Cm=scene.vec(actor['p'].get('location')).tolist(),
            initialEulerDegrees=[rotation.get(k, 0.) for k in ['roll', 'pitch', 'yaw']],
            scaleUE3=[1., 1., 1.],
            positionKeys=transform_keys(move['p']['postrack']['points'], move['name'], 'position'),
            eulerKeys=transform_keys(move['p']['eulertrack']['points'], move['name'], 'euler'))
        detail = imported.default_detail()
        detail['timing'].update(startDelaySeconds=0., lifeTimeSeconds=DURATION)
        detail['particle']['initialPositionMin'] = [0., 0., 0.]
        detail['particle']['initialPositionMax'] = [0., 0., 0.]
        color = props.get('lightcolor', defaults['color'])
        detail['light'].update(enabled=True, profileId=f'light.{kind}.reconstructed.v1',
            status='reconstructed_profile', range=float(props.get('radius', defaults['radius']))*.01,
            intensity=1., specularIntensity=1., color=[color[k]/255. for k in 'rgb']+[1.],
            ambient=[0., 0., 0., 0.], falloffExponent=float(props.get('falloffexponent', defaults['falloffexponent'])))
        if kind != 'point':
            detail['light']['direction'] = [1., 0., 0.]
        if kind == 'spot':
            # Spot CDO export 12479 omits InnerConeAngle; Epic UE3 LightingReference
            # documents native default 1 degree. Instance supplies OuterConeAngle=10.
            detail['light'].update(innerConeDegrees=float(props.get('innerconeangle', 1.)),
                                   outerConeDegrees=float(props['outerconeangle']))
        stable = f'kouku.gate1.popup.light.{group_id}'
        brightness = distribution(rows[track_id]['p']['floattrack']['points'],
                                  rows[track_id]['name'], 'coloroverlife', normalized=True)
        elements.append(dict(id=stable, displayName=f"SCENE03A {rows[group_id]['p']['groupname']} {kind}",
            groupId=ASSET, sourceNode='', visible=True, kind='light', resources=[],
            transformInheritance=dict(enabled=False, masterElementId=''),
            material=dict(templateId='effect.standard', sourceMaterialPath='',
                renderProfile='alpha_two_sided_depth_read', sourceProfile=dict(enabled=False)),
            detail=detail, sourceRecipe=dict(enabled=True, rendererShape='light',
                emitterDelaySeconds=0., emitterDurationSeconds=DURATION, emitterLoopCount=1, bursts=[],
                modules=[dict(stableId=stable+'.brightness', className='particlemodulecoloroverlife',
                    objectPath=rows[track_id]['name'], literals=[], distributions=[brightness])]),
            sourceTransformTrack=dict(sourceOccurrenceId=stable, sourceTimeOriginSeconds=0.,
                                      previewOriginUE3Cm=[0., 0., 0.], nodes=[node]),
            sourcePresentation=imported.default_source_presentation()))
        receipts.append(dict(group=group_id, actor=actor_id, component=component_id,
            brightnessTrack=track_id, moveTrack=move['index'], kind=kind,
            sourceProperties=copy.deepcopy(props), detail=detail['light'],
            transformFrame=frame, specularBasis='Movable source light color times brightness; no separate specular override is serialized',
            innerConeBasis='Instance' if 'innerconeangle' in props else 'Epic UE3 LightingReference documented native default 1 degree',
            receiver='ALL', receiverBasis='Inherited Engine LightComponent channels enable BSP, Static, Dynamic and CompositeDynamic'))
    assert len(elements) == 7 and len(unbound) == 1
    document = dict(schema='lostark.effect-authoring', version=13, effectAssetId=ASSET,
        displayName='1관문_팝업북 원본 이동·밝기 조명',
        particleSystem=dict(uniformScaleMultiplier=1, yawOffsetDegrees=0,
                            directionYawDegrees=0, initialSpeedMultiplier=1),
        modelCues=[], elements=elements)
    target = destination/f'Data/Effects/Authored/{ASSET}.effect.json'
    scene.write(target, document)
    report = dict(effectAssetId=ASSET, candidate=str(target), source=str(source),
        sourceSha256=hashlib.sha256(raw).hexdigest(), sourceUnchanged=source.read_bytes() == raw,
        sourceMatinee=365, sourceInterpData=857, lightCount=len(elements),
        placement=dict(startMs=12258, durationMs=46552, position=[0., 0., 0.],
                       rotationDegrees=[0., 0., 0.], scale=[1., 1., 1.]),
        landmarks=[dict(sourceSeconds=a, authoredSeconds=b+START_SECONDS) for a, b in LANDMARKS],
        lights=receipts, unboundTracks=unbound, classDefaultEvidence=provenance,
        nativeDefaultReference='https://docs.unrealengine.com/udk/Three/LightingReference.html',
        remainingBoundary='Fine named channel masks, light shafts and dynamic shadow maps are not represented; '
            'existing transient LIGHT diffuse/attenuation/direction carrier is used.')
    scene.write(destination/'popup-light-projection.json', report)
    print(json.dumps(dict(effectAssetId=ASSET, elements=len(elements), unbound=len(unbound), candidate=str(target))))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, default=ROOT/'out/KoukuSourceSequenceRestore20260912/LV_LUT_MIDNIGHTC_ED_SCENE03A.json')
    parser.add_argument('--candidate-root', type=Path, default=ROOT/'out/KoukuPattern3Sequence20260913/candidate')
    args = parser.parse_args()
    build(args.source, args.candidate_root)
