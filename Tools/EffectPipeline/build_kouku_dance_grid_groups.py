"""Compose installed Kouku V1 leaves into root dance and split fire-grid groups.

No source material, Cascade module, geometry or shader is regenerated here.
Dance is explicitly authored at the occurrence root, without fictitious sockets.
Grid placement comes from original Action/SkillEffect and decoded Projectile data.
"""
import argparse
import base64
import copy
import hashlib
import json
import math
from pathlib import Path
import sqlite3
import struct

ROOT = Path(__file__).resolve().parents[2]
SOURCE = Path('C:/LostArkExtract/LV_LUT_MIDNIGHTC_ED_20260829')
AUTHORED = ROOT / 'Data/Effects/Authored'
GRID_CACHE = ROOT / 'out/KoukuRainbowMatched20260911/decoded_grid.json'
ACTION = SOURCE / 'RemainingCharacterExtraction-20260829/ActionNameSources/MN_RPCT_07.action-effects.json'
DATABASE = SOURCE / 'WorldObjectExtraction-20260907/EFTable_SkillEffect.db'
SYSTEMS = {
    'aura': 'fx_mn_rpct_05_g.par_g_rpct_05_dance_aura_01_loc_int',
    'light': 'fx_mn_rpct_05_g.par_g_rpct_05_dance_light_01',
    'warning': 'fx_mn_rpct_07_v.par_v_rpct_fire_decal_t_01_loc_int',
    'wave': 'fx_mn_rpct_07_v.par_v_rpct_fire_bigearthwave_01_loc_int',
    'flash': 'fx_cm_02.light.par_mp_light_01',
}


def read(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + '\n', encoding='utf-8')


def leaf(role):
    name = ('effect.kouku.gate3.rainbow.fire.light.full.restore' if role == 'flash'
            else 'effect.kouku.source.' + SYSTEMS[role])
    return read(AUTHORED / (name + '.effect.json'))


def remap(value, ids):
    if isinstance(value, dict):
        return {key: remap(item, ids) for key, item in value.items()}
    if isinstance(value, list):
        return [remap(item, ids) for item in value]
    return ids.get(value, value) if isinstance(value, str) else value


def append_leaf(document, original, call, index):
    group = 'kouku.group.' + hashlib.sha256((document['effectAssetId'] + '/' + str(index)).encode()).hexdigest()[:20]
    ids = {row['id']: group + '.' + str(i) for i, row in enumerate(original['elements'])}
    for row in original['elements']:
        element = remap(copy.deepcopy(row), ids)
        element['groupId'] = group
        # The enclosing authored occurrence supplies BOSS/root ownership.
        attachment = element['actionCueAttachment']
        attachment.update(enabled=False, follow=False, sourceAnchorSlotId='root',
                          runtimeAnchorSlotId='root', runtimeBoneName='')
        attachment.pop('snapshotRootSourceBasisYawDegrees', None)
        attachment['socketLocalTransform'] = dict(position=[0, 0, 0], rotationDegrees=[0, 0, 0], scale=[1, 1, 1])
        element['detail']['transform'].update(position=call['positionMeters'],
            rotationDegrees=[0, call['clientYawDegrees'], 0], scale=[1, 1, 1])
        element['detail']['timing']['startDelaySeconds'] += call['startSeconds']
        element['sourceNode'] = call['sourceEventId'] + '|' + element['sourcePresentation']['sourceObjectPath']
        element['sourcePresentation'].update(sourceActionCueId=call['sourceEventId'],
            sourceEventId=call['sourceEventId'], sourceOccurrenceIndex=index,
            sourceTimeSeconds=call['sourceStartSeconds'])
        document['elements'].append(element)


def compose(asset_id, name, calls, leaves):
    document = copy.deepcopy(leaves[calls[0]['role']])
    document.update(effectAssetId=asset_id, displayName=name, elements=[], modelCues=[])
    document['particleSystem'].update(uniformScaleMultiplier=1, yawOffsetDegrees=0,
                                     directionYawDegrees=0, initialSpeedMultiplier=1)
    origin = min(call['sourceStartSeconds'] for call in calls)
    for index, call in enumerate(calls):
        call['startSeconds'] = call['sourceStartSeconds'] - origin
        append_leaf(document, leaves[call['role']], call, index)
    assert min(row['detail']['timing']['startDelaySeconds'] for row in document['elements']) == 0
    return document, origin


def grid_occurrences(action, stage_index, connection, projectiles):
    stage = next(s for s in action['stages'] if s['stageIndex'] == stage_index)
    result = []
    for notify in stage['notifies']:
        if notify['sourceType'] != 'Effect':
            continue
        payload = base64.b64decode(notify['serializedPayload']['data'])
        key = struct.unpack_from('<i', payload, 90)[0]
        rows = connection.execute('select PrimaryKey,SecondaryKey,Key,ValueA,AreaOffsetX,AreaOffsetY,AreaOffsetZ,AreaOffsetAngle from SkillEffect where PrimaryKey=?', (key,)).fetchall()
        for found in rows:
            row = dict(found)
            if row['Key'] != 12 or str(row['ValueA']) not in projectiles:
                continue
            result.append(dict(notifyId=notify['notifyId'], notifySeconds=notify['localTimeSeconds'],
                sourceSkillEffect=row, projectileId=str(row['ValueA'])))
    return result


def grid_calls(occurrences, projectiles):
    calls = []
    roles = {SYSTEMS[role]: role for role in ('warning', 'wave', 'flash')}
    for occurrence in occurrences:
        row = occurrence['sourceSkillEffect']
        for index, particle in enumerate(projectiles[occurrence['projectileId']]['particles']):
            assert particle['sourcePositionCm'] == [0, 0, 0] and particle['sourceScale'] == [1, 1, 1]
            role = roles[particle['sourceParticleSystem']]
            if role != 'flash':
                assert all(p['name'].lower() == 'none' for p in particle['parameterOverrides'])
            calls.append(dict(role=role, sourceStartSeconds=occurrence['notifySeconds'] + particle['timerSeconds'],
                positionMeters=[row['AreaOffsetX'] * .01, row['AreaOffsetZ'] * .01, -row['AreaOffsetY'] * .01],
                clientYawDegrees=row['AreaOffsetAngle'] + particle['sourceRotationDegrees'][1],
                sourceEventId=occurrence['notifyId'] + '/projectile-' + occurrence['projectileId'] + '/particle-' + str(index),
                sourceParticleSystem=particle['sourceParticleSystem'], sourceParticleByteOffset=particle['sourceByteOffset'],
                sourceProjectileId=occurrence['projectileId'], sourceSkillEffect=row))
    return calls


def validate(document):
    rows = document['elements']
    assert len({row['id'] for row in rows}) == len(rows)
    assert len(document['displayName'].encode('utf-8')) <= 64
    encoded = json.dumps(document, allow_nan=False)
    assert 'B_WP_3' not in encoded and 'B_WP_4' not in encoded and 'B_WP_5' not in encoded
    paths = set()
    def visit(value):
        if isinstance(value, dict):
            for item in value.values():
                visit(item)
        elif isinstance(value, list):
            for item in value:
                visit(item)
        elif isinstance(value, str) and value.startswith('Effect/'):
            assert '..' not in Path(value).parts and ':' not in value
            paths.add(value)
    visit(document)
    missing = sorted(path for path in paths if not (ROOT / 'Client/Bin/Resources' / path).is_file())
    assert not missing, missing
    duration = max(row['detail']['timing']['startDelaySeconds'] + row['detail']['timing']['lifeTimeSeconds'] +
        (0 if row['kind'] == 'light' else max(row['detail']['particle']['lifeTimeSeconds'])) for row in rows)
    return dict(elementCount=len(rows), durationMs=math.ceil(duration * 1000), resourceCount=len(paths),
                missingResources=missing, uniqueElementIds=True, standaloneStartSeconds=0)


def build(evidence, install=False):
    cache = read(GRID_CACHE)
    projectiles = cache['projectiles']
    leaves = {role: leaf(role) for role in SYSTEMS}
    assert len(leaves['aura']['elements']) == 7 and len(leaves['light']['elements']) == 3
    documents, entries = [], []
    def add(asset, name, parent, calls, basis):
        document, origin = compose(asset, name, copy.deepcopy(calls), leaves)
        checked = validate(document)
        documents.append(document)
        entries.append(dict(id=asset, name=name, treeparent=parent,
            effectAssetId=asset, displayName=name,
            categoryPath=['KoukuSaydon', '1관문' if parent == '무지개 댄스' else '3관문',
                          '패턴', '세이튼', '무지개댄스' if parent == '무지개 댄스' else '화염격자'],
            defaultAnchorKind='BOSS' if parent == '무지개 댄스' else 'MAP',
            path='Data/Effects/Authored/' + asset + '.effect.json', durationMs=checked['durationMs'],
            previewAnchor='BOSS', followBoss=parent == '무지개 댄스', sourceTimeOriginSeconds=origin,
            source=basis, calls=calls, validation=checked, manualVisualValidation='USER_PENDING'))
    dance_calls = [dict(role=role, sourceStartSeconds=0, positionMeters=[0, 0, 0], clientYawDegrees=0,
        sourceEventId='authored-root/rainbow-dance/' + role, sourceParticleSystem=SYSTEMS[role])
        for role in ('aura', 'light')]
    dance_basis = dict(kind='EXPLICIT_BOSS_ROOT_AUTHORING', sourceParticleSystems=[SYSTEMS['aura'], SYSTEMS['light']],
        placement='Original leaf local coordinates retained at BOSS occurrence root; no B_WP_3/4/5 binding claimed.',
        timing='Independent root composition starts at zero; original Cascade modules and lifetimes retained.')
    for suffix, name, calls in (
        ('aura', '무지개댄스_오라_루트배치', dance_calls[:1]),
        ('light', '무지개댄스_조명3_루트배치', dance_calls[1:]),
        ('full', '무지개댄스_전체_루트배치', dance_calls)):
        add('effect.kouku.dance.' + suffix + '.root', name, '무지개 댄스', calls, dance_basis)
    actions = read(ACTION)['actions']
    with sqlite3.connect(DATABASE.as_uri() + '?mode=ro', uri=True) as connection:
        connection.row_factory = sqlite3.Row
        for variant, action_id, stage_index, count, angles, label in (
            ('a', 4219914, 0, 5, {0}, 'A평행'),
            ('b', 4219915, 1, 5, {90}, 'B직교5줄'),
            ('c', 4219916, 2, 10, {45, 135}, 'C대각')):
            action = next(a for a in actions if a['actionId'] == action_id)
            occurrences = grid_occurrences(action, stage_index, connection, projectiles)
            assert len(occurrences) == count, (variant, len(occurrences))
            assert {o['sourceSkillEffect']['AreaOffsetAngle'] for o in occurrences} == angles
            calls = grid_calls(occurrences, projectiles)
            basis = dict(kind='ORIGINAL_ACTION_SKILLEFFECT_PROJECTILE', actionId=action_id, stageIndex=stage_index,
                actionSource=str(ACTION), skillEffectDb=str(DATABASE), projectileSource=str(GRID_CACHE), occurrences=occurrences)
            if variant == 'b':
                basis['stageBoundary'] = ('This is stage001 perpendicular five-line pass. '
                    'The earlier stage000 parallel pass is available as variant A; '
                    'no unverified cross-stage transition delay is invented.')
            for phase, phase_name in (('warning', '예고'), ('impact', '폭발')):
                selected = [call for call in calls if (call['role'] == 'warning') == (phase == 'warning')]
                add('effect.kouku.firegrid.' + variant + '.' + phase, '광기의불길_' + label + '_' + phase_name,
                    '광기의 불길', selected, basis)
    # A single complete line is the two opposed ParticleSystem calls of one FixArea.
    occurrence = dict(notifyId='standalone-line/421991401', notifySeconds=0, projectileId='421991401',
        sourceSkillEffect=dict(PrimaryKey=None, AreaOffsetX=0, AreaOffsetY=0, AreaOffsetZ=0, AreaOffsetAngle=0))
    calls = grid_calls([occurrence], projectiles)
    for phase, phase_name in (('warning', '예고'), ('impact', '폭발')):
        selected = [call for call in calls if (call['role'] == 'warning') == (phase == 'warning')]
        add('effect.kouku.firegrid.line.' + phase, '광기의불길_한줄_' + phase_name, '광기의 불길', selected,
            dict(kind='ORIGINAL_PROJECTILE_LINE_AT_ROOT', projectileId=421991401, projectileSource=str(GRID_CACHE)))
    staged = []
    for document in documents:
        path = AUTHORED / (document['effectAssetId'] + '.effect.json')
        before = path.read_bytes() if path.exists() else None
        assert before is None or json.loads(before) == document, 'Preserve authored edits: ' + str(path)
        staged.append((path, before, document))
    for path, before, document in staged:
        assert (path.read_bytes() if path.exists() else None) == before, 'Concurrent edit: ' + str(path)
        write(evidence / 'candidate' / path.name, document)
        if install and before is None:
            write(path, document)
    manifest = dict(schema='lostark.kouku-pattern-group-installation', installed=install,
        documents=entries, preservedSourceLeaves=True, sourceAssetsRebuilt=False,
        productBuildRun=False, clientOrServerRun=False)
    write(evidence / 'manifest.json', manifest)
    print(json.dumps(dict(installed=install, documents=[{key: e[key] for key in ('id', 'durationMs')} for e in entries])))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--evidence-root', type=Path, default=ROOT / 'out/KoukuPatternGroups20260912/dance_grid')
    parser.add_argument('--install', action='store_true')
    args = parser.parse_args()
    build(args.evidence_root, args.install)
