"""Prepare gun-local and floor Showtime variants without overwriting live drafts.

The original ParticleSystem library stays unchanged.  Candidate occurrences use
the existing WORLD gun owner or the user's saved MAP point; no projectile flight
or original callback timing is invented by this authoring repair.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
from pathlib import Path
import shlex
import struct
import sys

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'Tools/ModelAssetConverter'))
from verify_dimensionmaster_summon_bind_pose import (
    MODEL_HEADER, SECTION_DESC, MESH_HEADER, SUBMESH_DESC, affine_matrix,
)

PREFIX = 'effect.kouku.gate3.showtime.'
GUN = 'Effect/KoukuSaydon/WorldObjects/SaydonShowtimeGun/SaydonShowtimeGun.wmodel'
COMPOSITION = 'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json'
WORLD = 'Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json'


def read(path):
    return json.loads(Path(path).read_text(encoding='utf-8-sig'))


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + '\n', encoding='utf-8')


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def measured_muzzle():
    path = ROOT / 'Client/Bin/Resources' / GUN
    data = path.read_bytes()
    header = MODEL_HEADER.unpack_from(data, 16)
    sections = [SECTION_DESC.unpack_from(data, 16 + MODEL_HEADER.size + i * SECTION_DESC.size)
                for i in range(header[1])]
    section, = [s for s in sections if s[0] == 1]
    offset = 16 + section[2] + 16
    mesh = MESH_HEADER.unpack_from(data, offset)
    assert mesh[0] == b'WMSH' and mesh[4] == 48
    offset += MESH_HEADER.size + mesh[1] * SUBMESH_DESC.size
    vertices = np.array([struct.unpack_from('<12f', data, offset + i * mesh[4]) for i in range(mesh[5])])
    # This installed mesh's front grille is the planar disc at Z=98.25 cm.
    # The furthest bounding vertex (Z=114.92) is an off-axis decoration.
    disc = np.unique(vertices[(vertices[:, 2] > 98) & (vertices[:, 2] < 98.5)
                              & (vertices[:, 5] > .98), :3], axis=0)
    assert len(disc) == 72, 'Reinspect the gun front grille after a model replacement'
    center = disc.mean(axis=0)
    _, _, basis = np.linalg.svd(disc - center)
    normal = basis[-1]
    if normal[2] < 0:
        normal = -normal
    error = float(np.max(np.abs((disc - center) @ normal)))
    assert error < .02 and normal[2] > .99
    # DirectX row-vector Roll -> Pitch -> Yaw, with pitch=0.  Original UE
    # ParticleSystem forward is +X; this adapter maps it to the gun disc normal.
    rotation = [0., math.degrees(math.atan2(-normal[2], normal[0])),
                math.degrees(math.asin(normal[1]))]
    return dict(modelAssetId=GUN, modelSha256=sha(path), vertices=mesh[5], indices=mesh[6],
                frontDiscVertexCount=len(disc), centerCm=center.tolist(), normal=normal.tolist(),
                coplanarityErrorCm=error, localPositionM=(center * .01).tolist(),
                sourceForwardToMuzzleRotationDegrees=rotation,
                basis='Installed front grille geometry; source weapon has no named muzzle socket')


def floor_sample(position):
    path = ROOT / 'Data/Navigation/LV_LUT_MIDNIGHTC_ED.navsource'
    with path.open(encoding='utf-8') as stream:
        h = shlex.split(next(stream))
        x = int((position[0] - float(h[6])) / float(h[5]))
        z = int((position[2] - float(h[7])) / float(h[5]))
        row = next(line.split() for line in stream if line.startswith(f'{x} {z} '))
    assert row[2:4] == ['1', '1']
    return dict(cell=[x, z], cellSizeM=float(h[5]), sampledHeightM=float(row[4]),
                savedPosition=position, fixedClearanceM=position[1] - float(row[4]),
                trackingEndClearanceBeforeM=position[1] - .1 - float(row[4]),
                clearanceAfterM=position[1] + .02 - float(row[4]),
                scope='Baked navigation sample; final rendered surface is user-verified')


def variant(slug, suffix, title, lift=None):
    document = read(ROOT / 'Data/Effects/Authored' / (PREFIX + slug + '.effect.json'))
    document['effectAssetId'] = PREFIX + slug + '.' + suffix
    document['displayName'] = title
    document.pop('sourceModelPreview', None)
    if slug == 'gun.signature':
        # Source group duplicates the same PS on B_WP_2 and B_WP_1.  One
        # independently anchored WORLD occurrence must contain just one hand.
        document['elements'] = [e for e in document['elements']
                                if e['actionCueAttachment']['runtimeBoneName'] == 'b_wp_2']
        assert len(document['elements']) == 11
    for element in document['elements']:
        if lift is None:
            element['actionCueAttachment'].update(enabled=False, follow=False)
            element['detail']['transform']['position'] = [0., 0., 0.]
            element['detail']['transform']['rotationDegrees'] = [0., 0., 0.]
        else:
            element['detail']['transform']['position'][1] += lift
    ids = {e['id'] for e in document['elements']}
    for e in document['elements']:
        for provider in e['sourceRecipe'].get('providerElementIds', []):
            assert provider in ids
    return document


def duration(document):
    return math.ceil(max(e['detail']['timing']['startDelaySeconds'] +
                         e['detail']['timing']['lifeTimeSeconds'] +
                         max(e['detail']['particle']['lifeTimeSeconds'])
                         for e in document['elements']) * 1000)


def prepare(output):
    source_path = ROOT / COMPOSITION
    composition = read(source_path)
    world = read(ROOT / WORLD)
    original = next(p for p in composition['patterns'] if p['patternId'] == 'KAKULSAYDON_G1_PATTERN_35')
    pattern = copy.deepcopy(original)
    resources = {r['assetId']: r for r in composition['presentationResources'] if r['kind'] == 'EFFECT'}
    by_occurrence = {o['occurrenceId'].rsplit('.', 1)[-1]: o for o in pattern['presentationOccurrences']}
    assert len(by_occurrence) == 7, 'Rebase candidate additions on the latest edited P35'
    muzzle = measured_muzzle()
    target_position = by_occurrence['7']['positionOffset'][:]
    floor = floor_sample(target_position)
    variants = [variant('gun.muzzle', 'world', '쇼타임 총구 발사 / WORLD 총구'),
                variant('gun.signature', 'world', '쇼타임 총구 섬광 / WORLD 총구'),
                variant('target.fixed', 'ground', '쇼타임 고정 표적 / MAP 바닥', .02),
                variant('target.tracking', 'ground', '쇼타임 노란 추적 표적 / MAP 바닥', .12),
                variant('target.end', 'ground', '쇼타임 노란 표적 종료 / MAP 바닥', .12)]
    added_resources = []
    for document in variants:
        path = output / 'effects' / (document['effectAssetId'] + '.effect.json')
        write(path, document)
        asset = document['effectAssetId']
        template = copy.deepcopy(resources[PREFIX + ('gun.muzzle' if '.gun.' in asset else 'target.fixed')])
        template.update(resourceId='kakulsaydon.effect.' + hashlib.sha256(asset.encode()).hexdigest()[:20],
                        displayName=document['displayName'], assetId=asset, durationMs=duration(document),
                        defaultAnchorKind='WORLD' if '.gun.' in asset else 'MAP')
        added_resources.append(template)
        resources[asset] = template
    added = []

    def append_occurrence(template, asset, start=None, ms=None):
        occurrence = copy.deepcopy(template)
        occurrence['occurrenceId'] = pattern['patternId'] + '.presentation.' + str(pattern['nextPresentationOccurrenceOrdinal'])
        pattern['nextPresentationOccurrenceOrdinal'] += 1
        occurrence['resourceId'] = resources[PREFIX + asset]['resourceId']
        if start is not None:
            occurrence['startMs'] = start
        if ms is not None:
            occurrence['durationMs'] = ms
        pattern['presentationOccurrences'].append(occurrence)
        added.append(occurrence)
        return occurrence

    gun_checks = []
    for ordinal, slug in [('2', 'gun.muzzle'), ('3', 'gun.signature')]:
        left = by_occurrence[ordinal]
        left.update(resourceId=resources[PREFIX + slug + '.world']['resourceId'], anchorKind='WORLD',
                    worldId=pattern['worldOccurrences'][0]['worldId'],
                    worldOccurrenceId=pattern['worldOccurrences'][0]['occurrenceId'], followBoss=True,
                    bone='', boneTarget='BODY', positionOffset=muzzle['localPositionM'][:],
                    rotationDegrees=muzzle['sourceForwardToMuzzleRotationDegrees'][:])
        right = append_occurrence(left, slug + '.world')
        right.update(worldId=pattern['worldOccurrences'][1]['worldId'],
                     worldOccurrenceId=pattern['worldOccurrences'][1]['occurrenceId'])
    by_occurrence['7'].update(resourceId=resources[PREFIX + 'target.fixed.ground']['resourceId'],
                              anchorKind='MAP', followBoss=False, worldId='', worldOccurrenceId='')
    # Preserve the user's existing seven start/duration windows.  Additional
    # target phases and impact are editable presentation timing, not decoded
    # original ProjectileTrace callback timing.
    target = by_occurrence['7']
    append_occurrence(target, 'target.tracking.ground', target['startMs'], 9500)
    append_occurrence(target, 'target.end.ground', target['startMs'] + 9500, 1200)
    append_occurrence(target, 'bullet', target['startMs'], 10000)
    append_occurrence(target, 'donut.warning.impact',
                      by_occurrence['5']['startMs'] + by_occurrence['5']['durationMs'], 6500)
    # Verify the meter-space muzzle is exactly the point rendered by the static
    # WModel preScale under every authored gun transform, including nonuniform
    # scale and user rotation. Bone/actor bases cancel from this local equality.
    for occurrence in pattern['worldOccurrences']:
        world_def = next(w for w in composition['worlds'] if w['worldId'] == occurrence['worldId'])
        instance = next(i for i in world['instances'] if i['instanceId'] == world_def['sequenceInstanceId'])
        obj = next(o for o in world['objectResources'] if o['objectId'] == instance['bindings'][0]['targetId'])
        sequence = next(t for t in world['templates'] if t['sequenceId'] == instance['templateId'])
        key = sequence['tracks'][0]['keys'][0]
        transform = np.array(affine_matrix(obj['scale'], key['rotationQuaternion'], key['positionOffset'])).reshape(4, 4)
        # A shared arbitrary affine witness models the later user placement and
        # actual skeletal basis. The same WORLD pivot consumer carries both.
        placement = occurrence['placement']
        scale = np.diag(placement['scale'] + [1.])
        transformed_mesh = np.r_[np.array(muzzle['centerCm']) * obj['modelPreScale'], 1.] @ transform @ scale
        transformed_effect = np.r_[muzzle['localPositionM'], 1.] @ transform @ scale
        error = float(np.max(np.abs(transformed_mesh - transformed_effect)))
        assert error < 1e-7
        gun_checks.append(dict(worldId=occurrence['worldId'], worldOccurrenceId=occurrence['occurrenceId'],
                               bone=obj['anchorBone'], modelPreScale=obj['modelPreScale'],
                               objectScale=obj['scale'], userPlacement=placement,
                               centerAgreementErrorM=error))
    catalog_additions = [dict(effectAssetId=d['effectAssetId'], payloadKind='DIRECT_AUTHORED_DOCUMENT',
                             authoringPath='Effects/Authored/' + d['effectAssetId'] + '.effect.json') for d in variants]
    patch = dict(sourcePath=COMPOSITION, sourceRevision=composition['revision'], sourceSha256=sha(source_path),
                 patternId=pattern['patternId'], originalPattern=original, pattern=pattern,
                 presentationResourcesToAdd=added_resources, effectCatalogEntriesToAdd=catalog_additions,
                 muzzle=muzzle, gunChecks=gun_checks, floor=floor,
                 changedExistingOccurrenceFields=['presentation.2/3: WORLD gun-local anchor and variant',
                                                 'presentation.7: MAP anchor and floor variant'],
                 timingBasis='Existing seven windows preserved; added windows are editable presentation authoring',
                 unresolved=['ProjectileTrace flight owner and callback timing remain undecoded; no flight path created',
                             'Original gun shell remains the source right-hand casing effect',
                             'Native library gun create/loop/end are not layered over the already visible WORLD gun meshes'],
                 visualValidation='USER_PENDING')
    write(output / 'showtime-anchor-authoring-patch.json', patch)
    for i, p in enumerate(composition['patterns']):
        if p['patternId'] == pattern['patternId']:
            composition['patterns'][i] = pattern
    composition['presentationResources'].extend(added_resources)
    composition['revision'] += 1
    write(output / 'KoukuSaydonComposition.showtime.json', composition)
    print(json.dumps(dict(variants=len(variants), addedOccurrences=len(added), muzzle=muzzle,
                         floor=floor, output=str(output)), ensure_ascii=False))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--output', type=Path, default=ROOT / 'out/KoukuPattern3Sequence20260913/candidate/showtime')
    prepare(parser.parse_args().output)
