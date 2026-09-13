"""Project the qualified SCENE03A static material carriers onto Gate 1's clock.

Writes candidates only. Original component material ownership and named Matinee
parameter curves stay attached to the same existing V1 mesh element.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
from pathlib import Path

import build_gate1_popup_lights as clock

ROOT = clock.ROOT
ASSET = 'effect.kouku.gate1.popup.source-material-carriers'
ACTORS = (672, 691, 692, 811, 812)


def build(source: Path, evidence: Path, destination: Path):
    raw = source.read_bytes()
    source_data = json.loads(raw)
    rows = {int(k): v for k, v in source_data['rows'].items()}
    inventory = clock.scene.read(evidence/'source_actor_inventory.json')
    qualified = clock.scene.read(evidence/'native/native_material_patch.json')['programs']
    materials = {occurrence: program for program in qualified for occurrence in program['occurrences']}
    mesh_materials = {r['elementId']: r['sourceMaterial'] for r in
                     clock.scene.read(evidence/'native/source_mesh_material_resolution.json')}
    geometry = {r['sourceObject']: r for r in clock.scene.read(evidence/'geometry_installation.json')}
    elements, carriers = [], []
    for record in inventory:
        actor_id = record['actorExport']
        if actor_id not in ACTORS:
            continue
        actor, component = record['sourceActor'], record['sourceComponent']
        assert actor == rows[actor_id] and component == rows[component['index']]
        assert not record['hidden'] and not actor['p'].get('base')
        for field in ('translation', 'rotation', 'scale', 'scale3d'):
            assert field not in component['p'], ('Unprojected component transform', field)
        move, = [r for r in record['sourceTracks'] if r['cls'] == 'interptrackmove']
        quaternion = bool(move['p'].get('busequatinterpolation', False))
        assert move['p'].get('moveframe', 'imf_world') == 'imf_world'
        assert all(p.get('groupname', 'none') == 'none'
                   for p in move['p'].get('lookuptrack', {}).get('points', []))
        stable = f'kouku.popup.carrier.{actor_id}'
        material = materials[stable]
        assert material['sourceMaterial'] == (record['sourceMaterial'] or mesh_materials[stable])
        scale = clock.scene.vec(actor['p'].get('drawscale3d', dict(x=1., y=1., z=1.)))
        scale *= float(actor['p'].get('drawscale', 1.))
        rotation = actor['p'].get('rotation', {}).get('degrees', {})
        node = dict(sourceObjectPath=actor['name'], frame='WORLD',
            initialPositionUE3Cm=clock.scene.vec(actor['p'].get('location')).tolist(),
            initialEulerDegrees=[rotation.get(k, 0.) for k in ('roll', 'pitch', 'yaw')],
            scaleUE3=scale.tolist(),
            positionKeys=clock.transform_keys(move['p']['postrack']['points'], move['name'], 'position'),
            eulerKeys=clock.transform_keys(move['p']['eulertrack']['points'], move['name'], 'euler'))
        if quaternion:
            # These two original intervals lie within one affine clock segment.
            # Preserve their endpoints for exact shortest-arc Slerp; interpolated
            # Euler keys or Hermite tangents would change the quaternion path.
            points = move['p']['eulertrack']['points']
            assert not any(points[0]['inval'] < t < points[-1]['inval']
                           for t, _ in clock.LANDMARKS)
            node['eulerKeys'] = [dict(timeSeconds=clock.retime(p['inval']),
                value=clock.scene.vec(p['outval']).tolist(), arriveTangent=[0., 0., 0.],
                leaveTangent=[0., 0., 0.], interpolation='linear') for p in points]
            node['useQuaternionInterpolation'] = True
        parameter_tracks, parameter_sources = [], []
        for track in record['sourceTracks']:
            if track['cls'] not in ('interptrackfloatmaterialparam', 'interptrackvectormaterialparam'):
                continue
            scalar = track['cls'] == 'interptrackfloatmaterialparam'
            props = track['p']
            references = [ref for material in props['materials'] for ref in material['affectedmaterialrefs']]
            assert references == [dict(primitive=component['index'], materialindex=0)]
            assert not any(m['affectedppchainmaterialrefs'] or m['affectedinterptrackmaterialrefs']
                           for m in props['materials'])
            points = props['floattrack' if scalar else 'vectortrack']['points']
            keys = clock.transform_keys(points, track['name'], props['paramname'])
            if scalar:
                for key in keys:
                    for field in ('value', 'arriveTangent', 'leaveTangent'):
                        key[field] = key[field][:1]
            parameter_tracks.append(dict(name=props['paramname'], kind='SCALAR' if scalar else 'VECTOR', keys=keys))
            parameter_sources.append(dict(track=track['index'], parameter=props['paramname'], sourcePoints=points))
        detail = clock.imported.default_detail()
        detail['timing'].update(startDelaySeconds=0., lifeTimeSeconds=clock.DURATION)
        detail['mesh'].update(modelPreScale=.01, useModelMaterial=False)
        detail['sprite']['billboard'] = False
        detail['particle']['billboard'] = False
        transform = dict(sourceOccurrenceId=stable, sourceTimeOriginSeconds=0.,
                         previewOriginUE3Cm=[0., 0., 0.], nodes=[node])
        if parameter_tracks:
            transform['materialParameterTracks'] = parameter_tracks
        elements.append(dict(id=stable, displayName=f"SCENE03A {rows[record['groupExport']]['p']['groupname']}",
            groupId=ASSET, sourceNode=component['name'], visible=True, kind='mesh',
            resources=[dict(slotId='meshModel', assetId=geometry[record['sourceMesh']]['assetId'])],
            transformInheritance=dict(enabled=False, masterElementId=''),
            material=copy.deepcopy(material['material']), detail=detail,
            sourceRecipe=dict(enabled=False, rendererShape='', emitterDelaySeconds=0.,
                              emitterDurationSeconds=0., emitterLoopCount=0, bursts=[], modules=[]),
            sourceTransformTrack=transform,
            sourcePresentation=clock.imported.default_source_presentation()))
        carriers.append(dict(actor=actor_id, component=component['index'], group=record['groupExport'],
            moveTrack=move['index'], nativeProgram=material['program'], sourceMaterial=material['sourceMaterial'],
            sourceMesh=record['sourceMesh'], materialParameterSources=parameter_sources,
            useQuaternionInterpolation=quaternion,
            visibility='No hidden flag or visibility track. Original movement and material opacity own visibility.'))
    # The ninth original curtain is attached to actor 819 through camera 4.
    # The eight editable map curtains own different source yaw occurrences;
    # keep them and restore this missing parent's original movement separately.
    curtain = clock.scene.read(evidence/'curtain848/source_actor_inventory.json')[0]
    assert curtain['actorExport'] == 848 and curtain['sourceActor'] == rows[848]
    component = curtain['sourceComponent']
    assert component == rows[component['index']] and curtain['sourceActor']['p']['base'] == 4
    stable = 'kouku.popup.carrier.848'
    material = materials[stable]
    assert material['sourceMaterial'] == curtain['sourceMaterial']
    transform = clock.scene.read(evidence/'curtain848/source_transform_track.json')
    assert transform['sourceOccurrenceId'] == stable
    assert [n['frame'] for n in transform['nodes']] == ['WORLD', 'PARENT', 'PARENT']
    assert transform['nodes'][0]['scaleUE3'] == transform['nodes'][1]['scaleUE3'] == [1., 1., 1.]
    mesh, = clock.scene.read(evidence/'curtain848/geometry_installation.json')
    assert mesh['sourceObject'] == curtain['sourceMesh']
    element = copy.deepcopy(elements[0])
    element.update(id=stable, displayName='SCENE03A U32 부착 커튼', sourceNode=component['name'],
        resources=[dict(slotId='meshModel', assetId=mesh['assetId'])],
        material=copy.deepcopy(material['material']), sourceTransformTrack=transform)
    elements.append(element)
    carriers.append(dict(actor=848, component=component['index'], group=None,
        parentActors=[819, 4], parentMoveTracks=[1349, 1458], nativeProgram=material['program'],
        sourceMaterial=material['sourceMaterial'], sourceMesh=curtain['sourceMesh'],
        useQuaternionInterpolation=True, materialParameterSources=[],
        visibility='Original attached curtain; source active-camera contribution and visibility links are recorded in curtain848/source_contribution.json.'))
    assert len(elements) == 6
    assert sum(len(e['sourceTransformTrack'].get('materialParameterTracks', [])) for e in elements) == 4
    document = dict(schema='lostark.effect-authoring', version=13, effectAssetId=ASSET,
        displayName='1관문_팝업북 원본 암전·섬광·빛기둥·바닥·커튼',
        particleSystem=dict(uniformScaleMultiplier=1., yawOffsetDegrees=0.,
                            directionYawDegrees=0., initialSpeedMultiplier=1.),
        modelCues=[], elements=elements)
    target = destination/f'Data/Effects/Authored/{ASSET}.effect.json'
    clock.scene.write(target, document)
    report = dict(effectAssetId=ASSET, candidate=str(target), source=str(source),
        sourceSha256=hashlib.sha256(raw).hexdigest(), sourceUnchanged=source.read_bytes() == raw,
        placement=dict(startMs=12258, durationMs=46552, anchorKind='MAP',
                       position=[0., 0., 0.], rotationDegrees=[0., 0., 0.], scale=[1., 1., 1.]),
        landmarks=[dict(sourceSeconds=a, authoredSeconds=b+clock.START_SECONDS) for a, b in clock.LANDMARKS],
        carriers=carriers, unresolved=[],
        quaternionReference='https://docs.unrealengine.com/udk/Three/MatineeTrackReference.html#Rotation%20Interpolation')
    clock.scene.write(destination/'popup-carrier-projection.json', report)
    print(json.dumps(dict(effectAssetId=ASSET, elements=len(elements), materialParameterTracks=4,
                          candidate=str(target))))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, default=ROOT/'out/KoukuSourceSequenceRestore20260912/LV_LUT_MIDNIGHTC_ED_SCENE03A.json')
    parser.add_argument('--evidence-root', type=Path, default=ROOT/'out/KoukuPattern3Sequence20260913/candidate/popup_carriers')
    parser.add_argument('--candidate-root', type=Path, default=ROOT/'out/KoukuPattern3Sequence20260913/candidate')
    args = parser.parse_args()
    build(args.source, args.evidence_root, args.candidate_root)
