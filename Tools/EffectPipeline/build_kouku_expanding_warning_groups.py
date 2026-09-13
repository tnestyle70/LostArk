"""Create and update warnings with fixed native boundaries and growing fill.

Only the missing inner-parameter timing is project authored. Original native
materials, fixed holes/boundaries, particle sizes, caustic animation and fades
stay intact. Existing .expand IDs remain compatible; the whole decal no longer
scales, which used to move both the safe-hole and outer boundaries together.
"""
import argparse
import copy
import hashlib
import json
from pathlib import Path
import sqlite3
import struct

import build_kouku_showtime_warning_groups as warning

ROOT = Path(__file__).resolve().parents[2]
AUTHORED = ROOT / 'Data/Effects/Authored'
OLD_SUFFIX = '_바깥확장(크기보간)'
NEW_SUFFIX = '_채움확장(고정경계)'
RADIAL_PROFILES = ('effect.ue3.kouku-3600-native.v1', 'effect.ue3.kouku-3601-native.v1')


def read(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2,
                               allow_nan=False) + '\n', encoding='utf-8')


def build(output):
    rows = []
    circus = []
    projectile = ROOT / 'out/KoukuAllEffects20260912/source/Projectile/421980613.loa'
    raw = projectile.read_bytes()
    assert struct.unpack_from('<f', raw, 2981)[0] == 2
    assert struct.unpack_from('<I', raw, 3262)[0] == 3
    database = warning.source.SOURCE / 'WorldObjectExtraction-20260907/EFTable_SkillEffect.db'
    with sqlite3.connect('file:' + database.as_posix() + '?mode=ro', uri=True) as connection:
        connection.row_factory = sqlite3.Row
        for shape, label, offset, decal, damage, inner, outer in (
                ('circle', '원형', 3266, 2112, 421980629, 0, 1),
                ('innerdonut', '도넛1', 3310, 2116, 421980630, 2.5, 4),
                ('outerdonut', '도넛2', 3354, 2116, 421980631, 5.5, 7)):
            assert struct.unpack_from('<II', raw, offset) == (decal, damage)
            area = dict(connection.execute('SELECT * FROM SkillEffect WHERE PrimaryKey=?', (damage,)).fetchone())
            assert (area['AreaRemoveRange'], area['AreaRange']) == (inner * 100, outer * 100)
            template = AUTHORED / ('effect.kouku.gate3.showtime.' + ('circle' if not inner else 'innerdonut') + '.warning.effect.json')
            asset = 'effect.kouku.common.circus.' + shape + '.warning'
            name = '쓰리투원투하_' + label + '_예고'
            doc = warning.independent_document(read(template), asset, name)
            warning.resize_warning(doc, outer, inner, 2)
            element = doc['elements'][0]
            element['displayName'] = name
            identity = 'project.groundeffect.adapter.circus.' + shape
            element['sourceNode'] = identity + '|' + element['material']['sourceMaterialPath']
            element['sourceTransformTrack']['sourceOccurrenceId'] = identity
            element['sourceTransformTrack']['nodes'][0]['sourceObjectPath'] = identity
            path = output / 'candidate' / (asset + '.effect.json')
            write(path, doc)
            category = ['KoukuSaydon', '1관문', '패턴', '세이튼', '쓰리투원투하', '원형·도넛']
            row = dict(effectAssetId=asset, displayName=name, path=path.relative_to(ROOT).as_posix(),
                durationMs=2000, defaultAnchorKind='MAP', categoryPath=category,
                sourceProjectileId=421980613, sourceSkillDecalId=decal, sourceDamageSkillEffectId=damage,
                sourceSha256=hashlib.sha256(raw).hexdigest(), sourcePath=projectile.relative_to(ROOT).as_posix(),
                finalInnerRadiusM=inner, finalOuterRadiusM=outer,
                fidelity='SOURCE_GROUNDEFFECT_MATERIAL_AND_AREA_WITH_AUTHORED_TIMELINE')
            rows.append(row)
            if inner:
                circus.append((doc, row))
    seeds = []
    for family, category in (
            ('gate1.blade-dance', ['1관문', '패턴', '세이튼', '칼날댄스', '원형·도넛']),
            ('gate3.showtime', ['3관문', '패턴', '세이튼', '쇼타임', '원형·도넛'])):
        for shape in ('innerdonut', 'outerdonut'):
            source_id = f'effect.kouku.{family}.{shape}.warning'
            source_path = AUTHORED / (source_id + '.effect.json')
            original = read(source_path)
            seeds.append((original, dict(sourceSha256=hashlib.sha256(source_path.read_bytes()).hexdigest(),
                sourcePath=source_path.relative_to(ROOT).as_posix(), categoryPath=['KoukuSaydon'] + category)))
    seeds.extend(circus)
    for original, source_row in seeds:
        source_id = original['effectAssetId']
        doc = copy.deepcopy(original)
        asset = source_id + '.expand'
        name = original['displayName'] + NEW_SUFFIX
        doc.update(effectAssetId=asset, displayName=name)
        assert len(doc['elements']) == 1 and not doc.get('modelCues')
        element = doc['elements'][0]
        assert element['kind'] == 'decal'
        detail = element['detail']
        assert detail['particle']['localSpace']
        assert detail['transform']['scale'] == [1, 1, 1]
        assert not detail['linearLerp']['scale']
        assert not element['actionCueAttachment']['enabled']
        assert not element['transformInheritance']['enabled']
        track = element['sourceTransformTrack']
        assert len(track['nodes']) == 1
        node = track['nodes'][0]
        assert node['initialPositionUE3Cm'] == [0, 0, 0]
        assert node['initialEulerDegrees'] == [0, 0, 0] and node['scaleUE3'] == [1, 1, 1]
        assert all(key['value'] == [0, 0, 0] for key in node.get('positionKeys', []))
        assert all(key['value'] == [0, 0, 0] for key in node.get('eulerKeys', []))
        params = {p['name']: p['value'] for p in element['material']['sourceProfile']['scalars']}
        start_scale = params['thickness']
        assert 0 < start_scale < 1
        # The original fade stays on the original element clock. Complete
        # growth before it fades away so the full size can actually appear.
        fade_keys = track['alphaScaleKeys']
        full_keys = [key['timeSeconds'] for key in fade_keys if key['value'][0] == 1]
        motion = max(full_keys) - detail['timing']['startDelaySeconds']
        assert 0 < motion <= detail['timing']['lifeTimeSeconds']
        element.update(id=asset + '.ring', groupId=asset, displayName=name)
        visible_life = detail['timing']['lifeTimeSeconds']
        policy = warning.animate_radial_fill(element)
        path = output / 'candidate' / (asset + '.effect.json')
        write(path, doc)
        rows.append(dict(effectAssetId=asset, displayName=name,
            path=path.relative_to(ROOT).as_posix(), durationMs=round(visible_life * 1000),
            defaultAnchorKind='MAP', categoryPath=source_row['categoryPath'],
            sourceEffectAssetId=source_id, sourceSha256=source_row['sourceSha256'],
            sourcePath=source_row['sourcePath'],
            **policy, motionSeconds=motion,
            finalOuterRadiusM=detail['decal']['size'][0] / 2,
            finalInnerRadiusM=detail['decal']['size'][0] / 2 * params['thickness']))
    manifest = dict(installed=False, documents=rows, manualVisualValidation='USER_PENDING')
    write(output / 'installation.json', manifest)
    print(json.dumps(dict(candidates=len(rows), installed=False)))
    return manifest


def radial_asset_ids():
    for family in ('gate1.blade-dance', 'gate3.showtime', 'common.circus'):
        for shape in ('circle', 'innerdonut', 'outerdonut'):
            yield f'effect.kouku.{family}.{shape}.warning'
            if shape != 'circle':
                yield f'effect.kouku.{family}.{shape}.warning.expand'
    for shape in ('circle', 'donut'):
        yield f'effect.kouku.gate3.showtime.{shape}.warning.impact'


def radial_fill_candidate(original):
    document = copy.deepcopy(original)
    elements = [e for e in document['elements']
                if e['material']['sourceProfile'].get('runtimeShaderProfileId') in RADIAL_PROFILES]
    assert len(elements) == 1, 'Expected one existing radial GroundEffect warning'
    element = elements[0]
    detail, track = element['detail'], element['sourceTransformTrack']
    native = element['material']['sourceProfile']['runtimeShaderProfileId']
    scalars = {p['name']: p['value'] for p in element['material']['sourceProfile']['scalars']}
    hole = float(scalars['thickness']) if native == RADIAL_PROFILES[1] else 0.0
    start = detail['timing']['startDelaySeconds'] + track['sourceTimeOriginSeconds']
    finish = max(k['timeSeconds'] for k in track['alphaScaleKeys'] if min(k['value']) >= 1)
    visible_life = max(k['timeSeconds'] for k in track['alphaScaleKeys']) - start
    removed_scale_lerp = detail['linearLerp']['scale']
    if removed_scale_lerp:
        assert document['effectAssetId'].endswith('.warning.expand')
        assert detail['transform']['scale'] == [hole, 1, hole]
        assert detail['linearLerp']['endScale'] == [1, 1, 1]
        assert abs(detail['timing']['lifeTimeSeconds'] - (finish - start)) < 1e-6
        detail['transform']['scale'] = [1, 1, 1]
        detail['linearLerp']['scale'] = False
        detail['timing']['lifeTimeSeconds'] = visible_life
    assert all(abs(value - visible_life) < 1e-6 for value in detail['particle']['lifeTimeSeconds'])
    assert abs(element['sourceRecipe']['emitterDurationSeconds'] - visible_life) < 1e-6
    old_tracks = [curve for curve in track.get('materialParameterTracks', []) if curve['name'] == 'inner']
    policy = warning.animate_radial_fill(element)
    new_tracks = [curve for curve in track['materialParameterTracks'] if curve['name'] == 'inner']
    assert not old_tracks or old_tracks == new_tracks, 'Preserve a user-authored inner curve'
    if document['effectAssetId'].endswith('.warning.expand'):
        document['displayName'] = document['displayName'].replace(OLD_SUFFIX, NEW_SUFFIX)
        element['displayName'] = element['displayName'].replace(OLD_SUFFIX, NEW_SUFFIX)
    unchanged_ids = [e['id'] for e in original['elements'] if e['id'] != element['id']]
    assert [e for e in original['elements'] if e['id'] in unchanged_ids] == [
        e for e in document['elements'] if e['id'] in unchanged_ids]
    radius = detail['decal']['size'][0] / 2
    policy.update(changedElementId=element['id'], preservedElementIds=unchanged_ids,
        elementCount=len(document['elements']), nativeParameterRow=0,
        nativeParameterLane=2 if native == RADIAL_PROFILES[1] else 1,
        fixedInnerRadiusM=hole * radius, fixedOuterRadiusM=radius,
        removedWholeProjectorScaleLerp=removed_scale_lerp,
        sourceTimeCurve='No serialized source curve found; only timing is project authored',
        boundaryPolicy='One original native draw retains fixed boundaries; no duplicated boundary layers')
    return document, policy


def stage_existing(output):
    rows = []
    for asset in radial_asset_ids():
        path = AUTHORED / (asset + '.effect.json')
        before = path.read_bytes()
        original = json.loads(before)
        assert original['effectAssetId'] == asset
        candidate, policy = radial_fill_candidate(original)
        staged = output / 'candidate' / path.name
        backup = output / 'BeforeInstall' / path.name
        backup.parent.mkdir(parents=True, exist_ok=True)
        if backup.exists():
            assert backup.read_bytes() == before, 'Preserve the initial staged baseline: ' + str(backup)
        else:
            backup.write_bytes(before)
        write(staged, candidate)
        rows.append(dict(effectAssetId=asset, target=path.relative_to(ROOT).as_posix(),
            candidate=staged.resolve().as_posix(), backup=backup.resolve().as_posix(),
            baselineSha256=hashlib.sha256(before).hexdigest(),
            candidateSha256=hashlib.sha256(staged.read_bytes()).hexdigest(),
            oldDisplayName=original['displayName'], displayName=candidate['displayName'], **policy))
    manifest = dict(installed=False, documents=rows, manualVisualValidation='USER_PENDING')
    write(output / 'radial_fill_installation.json', manifest)
    print(json.dumps(dict(candidates=len(rows), installed=False)))
    return manifest


def install_existing(output):
    manifest = read(output / 'radial_fill_installation.json')
    for row in manifest['documents']:
        assert hashlib.sha256((ROOT / row['target']).read_bytes()).hexdigest() == row['baselineSha256'], \
            'Authored changed after staging: ' + row['target']
        assert hashlib.sha256(Path(row['candidate']).read_bytes()).hexdigest() == row['candidateSha256']
        assert hashlib.sha256(Path(row['backup']).read_bytes()).hexdigest() == row['baselineSha256']
    replaced = []
    try:
        for row in manifest['documents']:
            target = ROOT / row['target']
            assert hashlib.sha256(target.read_bytes()).hexdigest() == row['baselineSha256']
            temporary = target.with_suffix(target.suffix + '.radial-fill.tmp')
            temporary.write_bytes(Path(row['candidate']).read_bytes())
            assert hashlib.sha256(target.read_bytes()).hexdigest() == row['baselineSha256']
            temporary.replace(target)
            replaced.append(row)
    except Exception as failure:
        rollback_errors = []
        for row in reversed(replaced):
            target = ROOT / row['target']
            try:
                assert hashlib.sha256(target.read_bytes()).hexdigest() == row['candidateSha256'], \
                    'Concurrent edit preserved; original baseline remains in ' + row['backup']
                temporary = target.with_suffix(target.suffix + '.radial-rollback.tmp')
                temporary.write_bytes(Path(row['backup']).read_bytes())
                assert hashlib.sha256(target.read_bytes()).hexdigest() == row['candidateSha256']
                temporary.replace(target)
            except Exception as rollback_failure:
                rollback_errors.append(row['target'] + ': ' + str(rollback_failure))
        manifest.update(installed=False, installFailure=str(failure), rollbackErrors=rollback_errors)
        write(output / 'radial_fill_installation.json', manifest)
        raise RuntimeError('Radial fill installation failed: ' + str(failure) +
                           '; rollback errors: ' + repr(rollback_errors)) from failure
    manifest['installed'] = True
    write(output / 'radial_fill_installation.json', manifest)
    return manifest


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--output', type=Path, default=ROOT / 'out/KoukuExpandingWarnings20260913')
    parser.add_argument('--stage-existing', action='store_true', help='Stage all 17 existing radial warning assets')
    parser.add_argument('--install-existing', action='store_true', help='Install an already validated staged batch')
    args = parser.parse_args()
    if args.install_existing:
        install_existing(args.output.resolve())
    elif args.stage_existing:
        stage_existing(args.output.resolve())
    else:
        build(args.output.resolve())
