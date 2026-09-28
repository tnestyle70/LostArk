"""Stage the original 421991210 rectangle warning and three impact occurrences.

The decoded source receipt owns original field offsets, times and area dimensions.
This composes existing LocalDecal/particle contracts; it never writes live drafts.
"""
import argparse
import copy
import hashlib
import json
from pathlib import Path
import struct

import build_kouku_showtime_warning_groups as warning
from build_kouku_backstep_flame_groups import inspect_document

ROOT = warning.ROOT
PREFIX = 'effect.kouku.gate3.showtime.rectangle.'
SOURCE_MATERIAL = 'fx_m_mi_o_00.fx_mi.fx_o_de_condsquare_02_01_tr'
NATIVE_ID = 'effect.ue3.kouku-3607-native.v1'
DEFAULT_OUTPUT = ROOT / 'out/KoukuShowtimeRectangle20260914'


def read(path):
    return json.loads(path.read_bytes())


def write(path, document):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes((json.dumps(document, ensure_ascii=False, indent=2, allow_nan=False) + '\n').encode('utf8'))


def verify_source(contract):
    """Recheck the exact decoded records against their source bytes before stage."""
    assert contract['schema'] == 'lostark.readonly-source-contract.showtime-rectangle'
    assert contract['version'] == 1
    raw = Path(contract['sourceProjectile']['path']).read_bytes()
    assert hashlib.sha256(raw).hexdigest() == contract['sourceProjectile']['sha256']
    assert len(raw) == contract['sourceProjectile']['byteSize'] == 10105
    for field in contract['startIndexDecal']['fields']:
        fmt = '<f' if field['type'] == 'FloatProperty' else '<I'
        assert struct.unpack_from(fmt, raw, field['byteOffset'])[0] == field['value']
    names = contract['startIndexDecal']['sourceNamedTiming']
    assert names == dict(Time=0.0, Duration=2.0, DecalBlendInTime=1.5,
                         DecalScaleTime=0.0, DecalFillTime=1.5, DecalBlendOutTime=0.5)
    impacts = [c for c in contract['calls'] if c['role'] == 'impact']
    assert len(impacts) == 3
    for index, call in enumerate(impacts):
        assert struct.unpack_from('<f', raw, call['timerValueByteOffset'])[0] == call['timeSeconds']
        offset = call['particleTokenByteOffset']
        count = struct.unpack_from('<I', raw, offset)[0]
        token = raw[offset + 4:offset + 3 + count].decode('ascii')
        assert token.split("'")[1].lower() == call['particleSystem'].lower()
        assert call['runtimeIndependentPositionM'] == [0, 0, (index - 1) * 6]
        assert call['sourceScale'] == [1, 1, 1] and call['runtimeIndependentYawDegrees'] == 0
    area = contract['areaRows'][0]
    assert (area['AreaRange'], area['AreaAngle'], area['AreaOffsetX']) == (1800, 300, -900)
    assert contract['geometry']['fullLengthM'] == 18 and contract['geometry']['fullWidthM'] == 3
    ground = contract['groundEffect']
    payload = Path(ground['path']).read_bytes()
    assert hashlib.sha256(payload).hexdigest() == ground['sha256']
    assert ground['sourceMaterial'] == SOURCE_MATERIAL
    assert list(struct.unpack_from('<4f', payload, ground['fieldOffsets']['activeColor'])) == ground['activeColor']
    return names, impacts


def key(time, values):
    return dict(timeSeconds=time, value=values, arriveTangent=[0] * len(values),
                leaveTangent=[0] * len(values), interpolation='linear')


def rectangle_warning(contract, native_material, template):
    """Keep the full projector fixed while the native inner parameter fills it."""
    name = '쇼타임 / 사각형 장판 | 쇼타임_사각형_예고'
    doc = warning.independent_document(template, PREFIX + 'warning', name)
    assert len(doc['elements']) == 1 and not doc.get('modelCues')
    element = doc['elements'][0]
    identity = 'project.groundeffect.adapter.showtime.rectangle'
    element.update(id=PREFIX + 'warning.decal', groupId=PREFIX + 'warning',
                   displayName='쇼타임_사각형_예고', sourceNode=identity + '|' + SOURCE_MATERIAL,
                   resources=[], material=copy.deepcopy(native_material))
    assert element['material']['sourceMaterialPath'] == SOURCE_MATERIAL
    assert element['material']['sourceProfile']['runtimeShaderProfileId'] == NATIVE_ID
    detail, recipe = element['detail'], element['sourceRecipe']
    width, length = contract['geometry']['fullWidthM'], contract['geometry']['fullLengthM']
    timing = contract['startIndexDecal']['sourceNamedTiming']
    duration = timing['Duration']
    detail['transform'].update(position=[0, 0, 0], rotationDegrees=[0, 0, 0], scale=[1, 1, 1])
    assert not any(detail['linearLerp'][field] for field in ('position', 'rotation', 'scale'))
    detail['timing'].update(startDelaySeconds=0, lifeTimeSeconds=duration)
    detail['color']['multiply'] = contract['groundEffect']['activeColor']
    detail['decal'].update(size=[width, length], depth=6, receiverMode='upwardSurfaces', normalCutoff=.5)
    detail['particle'].update(lifeTimeSeconds=[duration] * 2, startSize=[width, length],
                              endSize=[width, length], localSpace=True)
    recipe.update(emitterDelaySeconds=0, emitterDurationSeconds=duration, emitterLoopCount=1)
    for module in recipe['modules']:
        for literal in module['literals']:
            if literal['propertyPath'] == 'emitterduration':
                literal['value'] = duration
            if literal['propertyPath'] == 'rotation.degrees.roll':
                literal['value'] = 0
        for distribution in module['distributions']:
            prop = distribution['propertyPath']
            if prop == 'lifetime':
                distribution.update(warning.constant_distribution(prop, [duration]))
            elif prop == 'startsize':
                # LocalDecal's particle Size.x/Size.y are the X/Z projector axes.
                distribution.update(warning.constant_distribution(prop, [width * 100, length * 100, width * 100]))
    track = element['sourceTransformTrack']
    track.update(sourceOccurrenceId=identity, sourceTimeOriginSeconds=0)
    assert len(track['nodes']) == 1 and track['nodes'][0]['scaleUE3'] == [1, 1, 1]
    track['nodes'][0]['sourceObjectPath'] = identity
    track['alphaScaleKeys'] = [key(t, [alpha] * 3) for t, alpha in
                              contract['startIndexDecal']['runtimeProjection']['alphaKeys']]
    track['materialParameterTracks'] = [dict(name='inner', kind='SCALAR',
        keys=[key(0, [0]), key(timing['DecalFillTime'], [1])])]
    overrides = dict(inner=0, decal_drawscale_x=width, decal_drawscale_y=length)
    profile = element['material']['sourceProfile']
    assert set(overrides) <= {p['name'] for p in profile['scalars']}
    for param in profile['scalars']:
        if param['name'] in overrides:
            param['value'] = overrides[param['name']]
    attachment = element['actionCueAttachment']
    assert not attachment['enabled'] and not element['transformInheritance']['enabled']
    attachment.pop('snapshotRootSourceBasisYawDegrees', None)
    return doc


def rectangle_impacts(contract, template):
    """Clone each original occurrence with its own IDs, offset and emission clock."""
    asset = PREFIX + 'impact'
    doc = copy.deepcopy(template)
    doc.update(effectAssetId=asset, displayName='쇼타임 / 사각형 장판 | 쇼타임_사각형_폭발', elements=[])
    assert len(template['elements']) == 16 and not template.get('sourceModelPreview')
    calls = [c for c in contract['calls'] if c['role'] == 'impact']
    first = calls[0]['timeSeconds']
    for index, call in enumerate(calls, 1):
        group = asset + '.burst' + str(index)
        own = warning.independent_document(template, group, '공습 폭발 ' + str(index))
        for element, original in zip(own['elements'], template['elements'], strict=True):
            # Preserve the original RNG identity through the portable-copy contract.
            origin = original['sourceNode']
            element['sourceNode'] = origin if origin.startswith('authored-copy:') else 'authored-copy:' + original['id']
            assert not element['actionCueAttachment']['enabled']
            assert not element['transformInheritance']['enabled']
            element['actionCueAttachment'].pop('snapshotRootSourceBasisYawDegrees', None)
            element['detail']['timing']['startDelaySeconds'] += call['timeSeconds'] - first
            element['detail']['transform'].update(position=call['runtimeIndependentPositionM'],
                rotationDegrees=[0, call['runtimeIndependentYawDegrees'], 0], scale=call['sourceScale'])
            element['sourcePresentation']['sourceTimeSeconds'] = call['timeSeconds']
            doc['elements'].append(element)
    assert len(doc['elements']) == 48
    assert len({e['id'] for e in doc['elements']}) == 48
    return doc


def stage(output):
    output = output.resolve()
    assert output.is_relative_to(ROOT / 'out'), 'Stage output must remain under out/'
    contract = read(output / 'source_contract.json')
    timing, calls = verify_source(contract)
    native_path = output / 'native/native_material_patch.json'
    material = next(p['material'] for p in read(native_path)['programs'] if p['program'] == 3607)
    authored = ROOT / 'Data/Effects/Authored'
    warning_path = authored / 'effect.kouku.gate3.showtime.circle.warning.effect.json'
    impact_path = authored / 'effect.kouku.gate3.showtime.airstrike.impact.effect.json'
    docs = [rectangle_warning(contract, material, read(warning_path)), rectangle_impacts(contract, read(impact_path))]
    entries = []
    for doc in docs:
        path = output / 'candidate' / (doc['effectAssetId'] + '.effect.json')
        write(path, doc)
        validation = inspect_document(doc)
        if doc['effectAssetId'].endswith('.warning'):
            validation['durationMs'] = round(timing['Duration'] * 1000)
        entries.append(dict(effectAssetId=doc['effectAssetId'], displayName=doc['displayName'],
            path=path.relative_to(ROOT).as_posix(), defaultAnchorKind='MAP', **validation))
    write(output / 'installation.json', dict(installed=False, stageOnly=True, documents=entries,
        sourceProjectileId=421991210, sourceSkillDecalId=2113, sourceWarningAreaSkillEffectId=421991224,
        geometry=contract['geometry'], sourceWarningTiming=timing,
        independentImpactStartSeconds=[c['timeSeconds'] - calls[0]['timeSeconds'] for c in calls],
        inputHashes={p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
                     for p in (warning_path, impact_path, native_path, output / 'source_contract.json')},
        compositionWritten=False, manualVisualValidation='USER_PENDING'))
    print(json.dumps(entries, ensure_ascii=False))


def stage_rectangle_missiles(output):
    """Add three aligned source missiles to the current saved impact group.

    This is an authored placement: remove the source horizontal scatter, use
    one missile per impact, and stop its flight when the impact begins. Keep
    the native mesh/material and -20 m/s^2 acceleration. Place the missile
    at 10 m so its one-second flight reaches the floor; retain alpha until
    impact instead of the source's pre-impact fade.
    Shift the existing group by one second; the composition installer must
    advance its occurrence by the same second to retain every impact time.
    """
    output = output.resolve()
    assert output.is_relative_to((ROOT / 'out').resolve())
    authored = ROOT / 'Data/Effects/Authored'
    impact_path = authored / (PREFIX + 'impact.effect.json')
    missile_path = authored / 'effect.kouku.gate3.showtime.airstrike.missile.effect.json'
    original, missile = read(impact_path), read(missile_path)
    doc = copy.deepcopy(original)
    assert len(doc['elements']) == 48 and len(missile['elements']) == 1
    assert not doc.get('modelCues') and not doc.get('sourceModelPreview')
    groups = {}
    for element in doc['elements']:
        assert '.burst' in element['groupId'], 'Preserve a changed user grouping'
        groups.setdefault(element['groupId'], []).append(element)
        element['detail']['timing']['startDelaySeconds'] += 1.0
    assert len(groups) == 3
    flights = []
    for index, (group_id, elements) in enumerate(groups.items(), 1):
        timing = min(e['detail']['timing']['startDelaySeconds'] for e in elements)
        position = elements[0]['detail']['transform']['position']
        assert all(e['detail']['transform']['position'] == position for e in elements)
        copied = warning.independent_document(missile, PREFIX + f'impact.missile{index}',
                                               f'공습 미사일 {index}')['elements'][0]
        copied['sourceNode'] = 'authored-copy:' + missile['elements'][0]['id']
        copied['detail']['transform'] = copy.deepcopy(elements[0]['detail']['transform'])
        copied['detail']['timing']['startDelaySeconds'] = timing - 1.0
        copied['detail']['particle']['lifeTimeSeconds'] = [1, 1]
        copied['detail']['particle']['initialPositionMin'] = [0, 10, 0]
        copied['detail']['particle']['initialPositionMax'] = [0, 10, 0]
        recipe = copied['sourceRecipe']
        recipe['bursts'] = [dict(timeSeconds=0, countMinimum=1, countMaximum=1)]
        for module in recipe['modules']:
            for distribution in module['distributions']:
                prop = distribution['propertyPath']
                if prop == 'lifetime':
                    distribution.update(warning.constant_distribution(prop, [1]))
                elif prop == 'startlocation':
                    value = [0, 0, 1000] if module['objectPath'].endswith('.particlemodulelocation_0') else [0, 0, 0]
                    distribution.update(warning.constant_distribution(prop, value))
                elif prop == 'alphascaleoverlife':
                    distribution.update(warning.constant_distribution(prop, [1]))
            for literal in module['literals']:
                if literal['propertyPath'] in ('burstlist[0].count', 'burstlist[0].countlow'):
                    literal['value'] = 1
        doc['elements'].append(copied)
        flights.append(dict(elementId=copied['id'], position=position,
                            flightStartSeconds=timing - 1.0, impactStartSeconds=timing))
    assert len({e['id'] for e in doc['elements']}) == 51
    candidate = output / 'candidate' / impact_path.name
    write(candidate, doc)
    checked = inspect_document(doc)
    write(output / 'rectangle-missiles.json', dict(stageOnly=True,
        inputHashes={p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
                     for p in (impact_path, missile_path)},
        candidate=candidate.relative_to(ROOT).as_posix(), flights=flights,
        compositionStartDeltaMs=-1000, compositionDurationDeltaMs=1000,
        existingImpactClockDeltaMs=1000, **checked))
    print(json.dumps(dict(candidate=str(candidate), flights=flights, **checked)))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--output', type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument('--add-missiles', action='store_true')
    args = parser.parse_args()
    (stage_rectangle_missiles if args.add_missiles else stage)(args.output)
