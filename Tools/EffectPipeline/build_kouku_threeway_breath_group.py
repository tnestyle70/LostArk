"""Restore Gate 3's staff, mouth and shoulder breath through the existing V1 path.

Source Action4219940 owns all timing, socket transforms and parameters. Native
materials come from the installed identical emitter streams, not name guesses.
Library registration is a separate install_kouku_effect_library operation.
"""
import argparse
import collections
import hashlib
import json
import math
from pathlib import Path

import build_kouku_gate1_full_restore as source
from extract_ue3_skeletal_mesh_sockets import parse_socket_contract

ROOT = source.ROOT
ASSET = 'effect.kouku.gate3.threeway.breath.full.restore'
ACTION = source.SOURCE / 'RemainingCharacterExtraction-20260829/ActionNameSources/MN_RPCT_07.action-effects.json'
SOCKETS = source.SOURCE / 'CanonicalSource/Character/UModelExports/MN_RPCT_05/Export/MN_RPCT_05/mesh/mn_rpct_05_sk.props.txt'
TEMPLATE = ROOT / 'Data/Effects/Authored/effect.kouku.gate1.4219801.full.restore.effect.json'


def project_rpct05_socket_basis(contract):
    """Project the four used UE sockets into this installed RPCT05 bone basis.

    PSK/WModel vertex and animated-bone measurements give the bind map
    0.01 * reflectZ. Combining UE export reflectY and the particle coordinate
    basis gives the local positions and exact DirectX rotations below. Keep
    this calibration local to RPCT05; the generic extractor owns source data.
    """
    rotations = {
        'fx_prj_01': ('b_wp_1', [49152, 0, 0], [-90, -90, 0]),
        'fx_prj_02': ('bip001-head', [0, 49152, 16384], [0, 180, 90]),
        'fx_prj_03': ('bip001-spine2', [0, 16384, 49152], [0, 0, -90]),
        'midcontrol': ('b_wp_1', [0, 0, 0], [-90, 0, 0]),
    }
    found = set()
    for socket in contract['sockets']:
        key = socket['socketName'].casefold()
        if key not in rotations:
            continue
        bone, source_rotation, runtime_rotation = rotations[key]
        transform = socket['sourceTransform']
        assert socket['boneName'].casefold() == bone, socket
        assert transform['rotationUnrealUnits'] == source_rotation, socket
        assert transform['scale'] == [1, 1, 1], socket
        x, y, z = transform['positionUeUnits']
        socket['runtimeLocalTransform'] = dict(
            position=[x * .01, -y * .01, -z * .01],
            rotationDegrees=runtime_rotation, scale=[1, 1, 1])
        found.add(key)
    assert found == rotations.keys(), found
    return contract


def build(evidence, install):
    evidence.mkdir(parents=True, exist_ok=True)
    sockets = parse_socket_contract(SOCKETS)
    source.write(evidence / 'source_socket_contract.before-basis.json', sockets)
    source.write(evidence / 'source_socket_contract.json', project_rpct05_socket_basis(sockets))
    old_action, old_selected = source.ACTION, source.SELECTED
    source.ACTION, source.SELECTED = ACTION, {4219940: ([0, 1], '3방향 불뿜기')}
    try:
        index, notifies, occurrences, records = source.acquire(evidence)
        template = source.read(TEMPLATE)
        materials = {}
        for element in template['elements']:
            key = (element['sourcePresentation']['sourceObjectPath'], element['material']['sourceMaterialPath'])
            assert key not in materials or materials[key] == element['material'], key
            materials[key] = element['material']
        programs = []
        for occurrence in occurrences:
            if occurrence['kind'] == 'light':
                continue
            key = (occurrence['sourceEmitter'], occurrence['sourceMaterial'])
            assert key in materials, ('Unrestored source emitter material', key)
            assert materials[key]['sourceProfile']['enabled'], key
            programs.append(dict(occurrences=[occurrence['elementId']], material=materials[key]))
        patch = evidence / 'native_material_patch.json'
        source.write(patch, dict(programs=programs))
        preview = source.source_model_preview(ACTION, 4219940, [0, 1], 'GATE3',
                                               'MN_RPCT_05', 'boss.kakulsaydon.g3.saydon')
        source.project(evidence, index, notifies, occurrences, records, evidence / 'projected',
                       material_patch=patch, source_model_previews={4219940: preview})
    finally:
        source.ACTION, source.SELECTED = old_action, old_selected
    document = source.read(evidence / 'projected/effect.kouku.gate1.4219940.full.restore.effect.json')
    document.update(effectAssetId=ASSET, displayName='3관문_쿠크세이튼_3방향불뿜기',
                    bloomIntensity=template.get('bloomIntensity', 1.0))
    by_anchor = collections.Counter(e['actionCueAttachment']['sourceAnchorSlotId'] for e in document['elements'])
    assert by_anchor['FX_Prj_01'] == by_anchor['FX_Prj_02'] == by_anchor['FX_Prj_03'] == 20, by_anchor
    shoulder = [e for e in document['elements'] if e['actionCueAttachment']['sourceAnchorSlotId'] == 'FX_Prj_03']
    assert all(e['actionCueAttachment']['runtimeBoneName'] == 'bip001-spine2' for e in shoulder)
    assert len(shoulder) == 20 and len(document['elements']) == 77
    candidate = evidence / 'candidate' / (ASSET + '.effect.json')
    source.write(candidate, document)
    target = ROOT / 'Data/Effects/Authored' / candidate.name
    if install:
        if target.exists():
            assert source.read(target) == document, 'Preserve authored tuning: ' + str(target)
        else:
            source.write(target, document)
    # The longest original impact tail is 11.074605 seconds; keep it in the
    # resource window instead of clipping it to the 4.834-second actor motion.
    duration = max(e['detail']['timing']['startDelaySeconds'] + e['detail']['timing']['lifeTimeSeconds'] +
                   e['detail']['timing']['afterImageSeconds'] + max(e['detail']['particle']['lifeTimeSeconds'])
                   for e in document['elements'])
    row = dict(effectAssetId=ASSET, displayName=document['displayName'],
        path=target.relative_to(ROOT).as_posix(), durationMs=math.ceil(duration * 1000),
        elementCount=len(document['elements']), defaultAnchorKind='BOSS', followBoss=True,
        categoryPath=['KoukuSaydon', '3관문', '패턴', '세이튼', '3방향 불뿜기'])
    source.write(evidence / 'installation.json', dict(installed=install, documents=[row],
        sourceActionId=4219940, sourceStageIndices=[0, 1], elementsByAnchor=dict(by_anchor),
        sourceInputs=[dict(path=str(p), sha256=hashlib.sha256(p.read_bytes()).hexdigest())
                      for p in (ACTION, SOCKETS, TEMPLATE)], manualVisualValidation='USER_PENDING'))
    print(json.dumps(dict(installed=install, elements=len(document['elements']), anchors=dict(by_anchor))))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--evidence-root', type=Path, default=ROOT / 'out/KoukuThreewayBreath20260912')
    parser.add_argument('--install', action='store_true')
    args = parser.parse_args()
    build(args.evidence_root, args.install)
