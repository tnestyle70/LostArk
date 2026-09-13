"""Compose the user's enlarged Saydon flame and the original upright hoop.

Installed source leaves own Cascade simulation, geometry and native materials.
Only this pattern's placement and shared flame scale are authored here. Catalog
and Composition registration use install_kouku_effect_library separately.
"""
import argparse
import copy
import hashlib
import json
import math
from pathlib import Path

import build_kouku_gate1_full_restore as source
import build_kouku_showtime_warning_groups as groups

ROOT = source.ROOT
AUTHORED = ROOT / 'Data/Effects/Authored'
PREFIX = 'effect.kouku.gate3.backstep.'
FLAME_SOURCE = 'effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_01_loc_int'
RING_SOURCE = 'effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_firering_01_loc_int'
RING_END_SOURCE = 'effect.kouku.source.fx_mn_rpct_05_g.par_g_rpct_05_firering_02_loc_int'
FLAME_SCALE = 2.0
RING_SCALE = 0.7
RING_HEIGHT = 1.1
RING_SOURCE_CENTER_CM = 225.0


def remap(value, identities):
    if isinstance(value, dict):
        return {key: remap(item, identities) for key, item in value.items()}
    if isinstance(value, list):
        return [remap(item, identities) for item in value]
    return identities.get(value, value) if isinstance(value, str) else value


def project_mesh_rotation(element):
    if element['sourceRecipe']['rendererShape'] != 'mesh':
        return
    modules = [m for m in element['sourceRecipe']['modules']
               if m['className'] == 'particlemoduletypedatamesh']
    assert len(modules) == 1, element['id']
    values = {literal['propertyPath']: literal['value'] for literal in modules[0]['literals']}
    rotation = [values.get(axis, 0.0) for axis in ('roll', 'pitch', 'yaw')]
    assert all(isinstance(v, (int, float)) and math.isfinite(v) for v in rotation)
    element['detail']['mesh']['sourceTypeDataRotationDegrees'] = rotation


def make_document(asset, name, calls):
    document = copy.deepcopy(calls[0]['leaf'])
    document.update(effectAssetId=asset, displayName=name, elements=[], modelCues=[])
    document['particleSystem'].update(uniformScaleMultiplier=1, yawOffsetDegrees=0,
                                     directionYawDegrees=0, initialSpeedMultiplier=1)
    document.pop('sourceModelPreview', None)
    document.pop('sourceAnchorAnimations', None)
    if document['version'] >= 15:
        document['runtimeExtensions'] = dict(formatVersion=1, bakedEdgeHistories=[])
    for call in calls:
        rows = call['leaf']['elements']
        selected = [e for e in rows if call['selection'] == 'all' or
                    (e['sourceRecipe']['rendererShape'] == 'decal') == (call['selection'] == 'ground')]
        group = 'kouku.backstep.' + hashlib.sha256((asset + '/' + call['role']).encode()).hexdigest()[:20]
        ids = {e['id']: group + '.' + str(i) for i, e in enumerate(selected)}
        for original in selected:
            element = remap(copy.deepcopy(original), ids)
            element['groupId'] = group
            attachment = element['actionCueAttachment']
            attachment.update(enabled=False, follow=False, sourceAnchorSlotId='root',
                              runtimeAnchorSlotId='root', runtimeBoneName='')
            attachment.pop('snapshotRootSourceBasisYawDegrees', None)
            attachment.pop('modelCueId', None)
            attachment['socketLocalTransform'] = dict(position=[0, 0, 0],
                rotationDegrees=[0, 0, 0], scale=[1, 1, 1])
            transform = element['detail']['transform']
            transform.update(position=call['position'], rotationDegrees=[0, call['yaw'], 0],
                             scale=[call['scale']] * 3)
            element['detail']['timing']['startDelaySeconds'] += call.get('start', 0)
            project_mesh_rotation(element)
            document['elements'].append(element)
    assert document['elements']
    assert len({e['id'] for e in document['elements']}) == len(document['elements'])
    assert len(name.encode('utf-8')) <= 64
    return document


def inspect_document(document):
    resources = set()
    def visit(value):
        if isinstance(value, dict):
            for item in value.values():
                visit(item)
        elif isinstance(value, list):
            for item in value:
                visit(item)
        elif isinstance(value, str) and value.startswith('Effect/'):
            assert '..' not in Path(value).parts and ':' not in value
            resources.add(value)
    visit(document)
    missing = [p for p in sorted(resources) if not (ROOT / 'Client/Bin/Resources' / p).is_file()]
    assert not missing, missing
    end = 0
    for element in document['elements']:
        assert element['material']['sourceProfile']['enabled']
        assert not element['actionCueAttachment']['enabled']
        timing, recipe = element['detail']['timing'], element['sourceRecipe']
        duration = timing['lifeTimeSeconds']
        if recipe['emitterDurationSeconds'] > 0 and recipe['emitterLoopCount']:
            duration = recipe['emitterDurationSeconds'] * recipe['emitterLoopCount']
        # Source decal recipes are particle simulations too; their particle tail
        # follows the same Calculate_ElementEndSeconds contract as sprite/mesh.
        tail = max(element['detail']['particle']['lifeTimeSeconds'])
        end = max(end, timing['startDelaySeconds'] + recipe['emitterDelaySeconds'] +
                  duration + timing['afterImageSeconds'] + tail)
    return dict(durationMs=math.ceil(end * 1000), elementCount=len(document['elements']),
                resourceCount=len(resources), missingResources=missing)


def build(evidence, install):
    leaves, inputs = {}, []
    for role, asset in [('flame', FLAME_SOURCE), ('ring', RING_SOURCE), ('end', RING_END_SOURCE)]:
        path = AUTHORED / (asset + '.effect.json')
        leaves[role] = source.read(path)
        inputs.append(dict(path=path.relative_to(ROOT).as_posix(), sha256=hashlib.sha256(path.read_bytes()).hexdigest()))
    assert len(leaves['flame']['elements']) == 14
    assert sum(e['sourceRecipe']['rendererShape'] == 'decal' for e in leaves['flame']['elements']) == 3
    assert len(leaves['ring']['elements']) == 10 and len(leaves['end']['elements']) == 7

    # Original Projectile421982502 supplies .7 scale and 1.1m height. Its +90
    # yaw cancels the common source-X to Client-forward -90 yaw. Move only the
    # independent pivot to the hoop centre, retaining relative smoke positions.
    ring = dict(role='ring', leaf=leaves['ring'], selection='all', scale=RING_SCALE,
                yaw=0, position=[0, RING_HEIGHT, RING_SOURCE_CENTER_CM * .01 * RING_SCALE])
    # Both usages consume this same call; no second ring-specific flame recipe.
    flame = dict(role='flame', leaf=leaves['flame'], selection='flame', scale=FLAME_SCALE,
                 yaw=-90, position=[0, RING_HEIGHT, 0])
    ground = dict(role='ground', leaf=leaves['flame'], selection='ground', scale=FLAME_SCALE,
                  yaw=-90, position=[0, 0, 0])
    end = dict(role='ring.end', leaf=leaves['end'], selection='all', scale=RING_SCALE,
               yaw=0, position=ring['position'])
    specs = [
        ('flame', '백스탭불뿜기_확대화염포', [flame]),
        ('ground', '백스탭불뿜기_바닥불길', [ground]),
        ('full', '백스탭불뿜기_화염포·바닥', [flame, ground]),
        ('ring', '화염링_생성·유지', [ring]),
        ('ring.flame', '화염링_동일화염포', [ring, flame]),
        ('ring.end', '화염링_원본종료잔불', [end]),
    ]
    documents, rows = [], []
    for suffix, name, calls in specs:
        document = make_document(PREFIX + suffix, name, calls)
        checked = inspect_document(document)
        source.write(evidence / 'candidate' / (document['effectAssetId'] + '.effect.json'), document)
        documents.append(document)
        rows.append(dict(effectAssetId=document['effectAssetId'], displayName=name,
            path='Data/Effects/Authored/' + document['effectAssetId'] + '.effect.json',
            categoryPath=['KoukuSaydon', '3관문', '패턴', '세이튼', '백스탭 불뿜기',
                          '화염링' if suffix.startswith('ring') else '화염포'],
            defaultAnchorKind='BOSS', followBoss=False, **checked))
    hoop = next(e for e in documents[3]['elements'] if e['sourceRecipe']['rendererShape'] == 'mesh')
    assert hoop['detail']['mesh']['sourceTypeDataRotationDegrees'] == [0, -90, 0]
    # Check the shared cannon's actual parameters, not merely its source ID.
    def cannon_payload(doc):
        result = []
        for element in doc['elements']:
            if element['sourcePresentation']['sourceObjectPath'].startswith('fx_mn_rpct_05_l.par_l_rpct_05_sk_01_loc_int.'):
                result.append({k: v for k, v in element.items() if k not in ('id', 'groupId')})
        return result
    assert cannon_payload(documents[0]) == cannon_payload(documents[4])
    assert len(cannon_payload(documents[0])) == 11
    if install:
        groups.install_independent_documents(documents)
    source.write(evidence / 'installation.json', dict(installed=install, documents=rows,
        sourceInputs=inputs, sourceProjectileId=421982502, sourceActions=[4219825, 4219951],
        sourceProjectileTransform=dict(positionUE3Cm=[120, 0, 110], rotationUnreal=[0, 16384, 0], scale=.7),
        sharedFlameSource=FLAME_SOURCE, sharedFlameScale=FLAME_SCALE,
        sharedFlameIdentityEqual=True, sourceTypeDataMeshRotationDegrees=[0, -90, 0],
        authoredPlacement='Forward +Z; hoop pivot centred above ground; flame origin equals hoop centre.',
        fidelityBoundary='Original native materials and modules; enlarged shared cannon and pivot placement are user-requested composition.',
        manualVisualValidation='USER_PENDING'))
    print(json.dumps(dict(installed=install, documents=len(rows), elements=sum(r['elementCount'] for r in rows))))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--evidence-root', type=Path, default=ROOT / 'out/KoukuSaitenGroups20260912/backstep')
    parser.add_argument('--install', action='store_true')
    args = parser.parse_args()
    build(args.evidence_root, args.install)
